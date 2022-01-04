//
// Created by luoxj@ihep.ac.cn on 2021/12/19.
//

#include "../PSDTools/PSDTool_sklearn.h"

#include "SniperKernel/ToolBase.h"
#include "SniperKernel/ToolFactory.h"
#include "SniperKernel/SniperPtr.h"


#include "RootWriter/RootWriter.h"


#include "SniperPython/PyDataStore.h"

#include "EvtNavigator/NavBuffer.h"
#include "TROOT.h"

namespace p = boost::python;
namespace np = boost::python::numpy;

using namespace std;
DECLARE_TOOL(PSDTool_sklearn);

PSDTool_sklearn::PSDTool_sklearn(const std::string &name): ToolBase(name){
    declProp("Path_Model", m_path_model );
    declProp("Output_Sklearn", m_output_file);
}

PSDTool_sklearn::~PSDTool_sklearn(){
}

bool PSDTool_sklearn::initialize(){
    LogInfo<<"Initializing PSDTool_sklearn..."<<std::endl;

    evtID = 0;

    // Initialization for python scripts
    np::initialize();
    m_ds = SniperDataPtr<PyDataStore>(m_par, "DataStore").data();
    m_ds->set("PSDVal", 0);

    // Initialize bins and histograms for PSD
    m_bins = {-19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
                7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46,
                48, 50, 52, 54, 56, 58, 60, 62, 66, 72, 80, 90, 102, 116, 132, 150, 170, 192, 216, 242, 270, 300, 332, 366,
                402, 440, 480, 522, 566, 612, 660, 710, 762, 816};
    m_h_time_without_charge = new TH1D("h_time_without_charge","h_time_without_charge", m_bins.size()-1, &(*m_bins.begin()) );
    m_h_time_with_charge = new TH1D("h_time_with_charge","h_time_with_charge", m_bins.size()-1, &(*m_bins.begin()) );

    //get the rootwriter
    SniperPtr<RootWriter> rwsvc(getParent(),"RootWriter");
    if ( rwsvc.invalid() ) {
        LogError << "cannot get the rootwriter service." << std::endl;
        return false;
    }

    //Store the pre-processed events
    gROOT->ProcessLine("#include <vector>");
    m_userTree=rwsvc->bookTree("evt", "evt");
    rwsvc->attach("USER_OUTPUT", m_userTree);
    m_userTree->Branch("evtID", &evtID, "evtID/I");
    m_userTree->Branch("h_time_without_charge", &v_h_time_without_charge );
    m_userTree->Branch("h_time_with_charge", &v_h_time_with_charge);
    m_userTree->Branch("XYZ", &vertex_xyz);
    m_userTree->Branch("Erec", &Erec, "Erec/D");

    //Store the PSDTools result, which may be implemented in data model in the future
    m_psdTree=rwsvc->bookTree("PSDTools", "PSDTools");
    m_userTree->Branch("evtID", &evtID, "evtID/I");
    m_psdTree->Branch("psdVal", &m_psdEvent.psdVal, "psdVal/D");
    m_psdTree->Branch("evtType", &m_psdEvent.evtType, "psdVal/I");
    rwsvc->attach("PSD_OUTPUT", m_psdTree);

//    SniperPtr<IPSDInputSvc> m_psdInput(getParent(), "PSDInputSvc");
    m_psdInput = dynamic_cast<IPSDInputSvc*>(getParent()->find("PSDInputSvc"));
    return true;
}

bool PSDTool_sklearn::finalize(){
    LogInfo<<"Finalizing PSDTool_sklearn..."<<std::endl;
    return true;
}

bool PSDTool_sklearn::preProcess( JM::EvtNavigator *nav){
    LogDebug<<"pre processing an event..."<<std::endl;
    evtID ++;
    if (!m_psdInput->extractHitInfo(nav,"alignPeak2")) return false;
    const vector<double> v_hittime = m_psdInput->getHitTime();
    const vector<double> v_charge = m_psdInput->getHitCharge();

    // Reset Variables
    m_h_time_with_charge->Reset();
    m_h_time_without_charge->Reset();

    // Fill histograms about time and charge
    for (int i=0;i<v_hittime.size();i++)
    {
        m_h_time_without_charge->Fill(v_hittime[i]);
        m_h_time_with_charge->Fill(v_hittime[i], v_charge[i]);
    }

    // Get Event Information
    vertex_xyz = m_psdInput->getEventXYZ();
    Erec = m_psdInput->getErec();


    v_h_time_with_charge.resize(m_bins.size()-1);
    v_h_time_without_charge.resize(m_bins.size()-1);
    for (int i=0;i<m_bins.size()-1;i++)
    {
        v_h_time_without_charge[i] = m_h_time_without_charge->GetBinContent(i+1);
        v_h_time_with_charge[i]    = m_h_time_with_charge->GetBinContent(i+1);
    }

    // Fill TTree for Training
    if (!b_usePredict) {
        m_userTree->Fill();
        evtID++;
    }
    else
    {
        // Set python array
        p::tuple shape = p::make_tuple(m_bins.size()-1);
        p::tuple shape_xyz_E = p::make_tuple(4);
        p::str path_model = p::str(m_path_model);

        np::dtype dtype_double = np::dtype::get_builtin<double>();

        np::ndarray arr_h_time_without_charge = np::zeros(shape, dtype_double);
        np::ndarray arr_h_time_with_charge = np::zeros(shape, dtype_double);
        np::ndarray arr_xyz_and_E = np::zeros(shape_xyz_E, dtype_double);

        for(int i=0;i<vertex_xyz.size();i++) arr_xyz_and_E[i] = vertex_xyz[i];
        arr_xyz_and_E[3] = Erec;

        for (int i=0;i<m_bins.size()-1;i++)
        {
            arr_h_time_without_charge[i] = v_h_time_without_charge[i];
            arr_h_time_with_charge[i]    = v_h_time_with_charge[i];
        }

        // register variables for python
        SniperDataPtr<PyDataStore> pystore(*getRoot(), "DataStore");
        if (pystore.invalid()) {
            LogError << "Failed to find the PyDataStore. Register the value to module " << std::endl;

            p::object this_module = p::import("PSDSklearn");
            this_module.attr("h_time_with_charge") = arr_h_time_with_charge;
            this_module.attr("h_time_without_charge") = arr_h_time_without_charge;
            this_module.attr("xyz_E") = arr_xyz_and_E;
            this_module.attr("path_model") = path_model;
            this_module.attr("output") = m_output_file;
        } else {
            LogInfo << "Register the value to PyDataStore. " << std::endl;
            pystore->set("h_time_with_charge", arr_h_time_with_charge);
            pystore->set("h_time_without_charge", arr_h_time_without_charge);
            pystore->set("xyz_E", arr_xyz_and_E);
            pystore->set("path_model", path_model);
            pystore->set("output", m_output_file);
        }

    }

    return true;
}

double PSDTool_sklearn::CalPSDVal(){
    m_ds->get("PSDVal",m_psdEvent.psdVal);
    cout<< "PSDVal:\t"<< m_psdEvent.psdVal << endl;
    m_psdEvent.evtType = EVTTYPE::Electron;
    m_psdTree->Fill();
    evtID ++;
    return m_psdEvent.psdVal;
}


