//
// Created by luoxj@ihep.ac.cn on 2021/12/19.
//

#include "PSDTools/PSDTool_sklearn.h"

#include "SniperKernel/ToolBase.h"
#include "SniperKernel/ToolFactory.h"
#include "SniperKernel/SniperPtr.h"
#include "SniperKernel/Task.h"


#include "RootWriter/RootWriter.h"


#include "SniperPython/PyDataStore.h"

#include "EvtNavigator/NavBuffer.h"
#include "TROOT.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "SniperKernel/Incident.h"

namespace p = boost::python;
namespace np = boost::python::numpy;

using namespace std;
DECLARE_TOOL(PSDTool_sklearn);

PSDTool_sklearn::PSDTool_sklearn(const std::string &name): ToolBase(name){
    declProp("Path_Model", m_path_model );
    declProp("Path_Bins", m_path_bins_file);
    declProp("PSD_divide", PSD_divide);
}

PSDTool_sklearn::~PSDTool_sklearn(){
}

bool PSDTool_sklearn::initialize(){
    LogInfo<<"Initializing PSDTool_sklearn..."<<std::endl;

    evtID = -1;

    // Initialization for python scripts
    np::initialize();

    // Get Sub Task from Sniper
    iotaskname = "TaskSklearn";
    sub_task = dynamic_cast<Task*>(getRoot()->find(iotaskname));

    // Initialize bins by loading txt file
    if (m_path_bins_file=="None")
    {
        m_bins = {-19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
                  7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46,
                  48, 50, 52, 54, 56, 58, 60, 62, 66, 72, 80, 90, 102, 116, 132, 150, 170, 192, 216, 242, 270, 300, 332, 366,
                  402, 440, 480, 522, 566, 612, 660, 710, 762, 816};
    }
    else
    {
        LogInfo << "Load Bins Setting from txt file" << endl;
        SetBins();
        cout<< "Check Loading Bins ---->" <<endl;
        for (auto &&bin:m_bins)
            cout<< bin << ",";
        cout<< endl;
    }

    // Initialize histograms for PSD
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
    m_userTree=rwsvc->bookTree(*m_par,"evt", "evt");
    rwsvc->attach("USER_OUTPUT", m_userTree);
    m_userTree->Branch("evtID", &evtID, "evtID/I");
    m_userTree->Branch("h_time_without_charge", &v_h_time_without_charge );
    m_userTree->Branch("h_time_with_charge", &v_h_time_with_charge);
    m_userTree->Branch("XYZ", &vertex_xyz);
    m_userTree->Branch("Erec", &Erec, "Erec/D");
//    m_userTree->Branch("PSDInput", &v_PSDInput);

    //Store the PSDTools result, which may be implemented in data model in the future
    m_psdTree=rwsvc->bookTree(*m_par,"PSDTools", "PSDTools");
    m_userTree->Branch("evtID_PSD", &evtID, "evtID_PSD/I");
    m_psdTree->Branch("psdVal", &m_psdEvent.psdVal, "psdVal/D");
    m_psdTree->Branch("evtType", &m_psdEvent.evtType, "evtType/I");
    rwsvc->attach("PSD_OUTPUT", m_psdTree);

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
//    v_PSDInput.resize((m_bins.size()-1)*2+2);
    for (int i=0;i<m_bins.size()-1;i++)
    {
        v_h_time_without_charge[i] = m_h_time_without_charge->GetBinContent(i+1);
        v_h_time_with_charge[i]    = m_h_time_with_charge->GetBinContent(i+1);

        // Set PSD Input
//        v_PSDInput[i] = m_h_time_without_charge->GetBinContent(i+1);
//        v_PSDInput[i+m_bins.size()-1] = m_h_time_with_charge->GetBinContent(i+1);
    }

    // Fill TTree for Training
    m_userTree->Fill();
    if (b_usePredict)
    {
        // Set python array
        p::tuple shape = p::make_tuple(m_bins.size()-1);
        p::tuple shape_xyz_E = p::make_tuple(4);
        p::tuple shape_PSDInput = p::make_tuple((m_bins.size()-1)*2 + 2 );
        p::str path_model = p::str(m_path_model);

        np::dtype dtype_double = np::dtype::get_builtin<double>();

        np::ndarray arr_h_time_without_charge = np::zeros(shape, dtype_double);
        np::ndarray arr_h_time_with_charge = np::zeros(shape, dtype_double);
        np::ndarray arr_xyz_and_E = np::zeros(shape_xyz_E, dtype_double);
        np::ndarray arr_PSDInput = np::zeros(shape_PSDInput, dtype_double);

        for(int i=0;i<vertex_xyz.size();i++) arr_xyz_and_E[i] = vertex_xyz[i];
        arr_xyz_and_E[3] = Erec;

        for (int i=0;i<m_bins.size()-1;i++)
        {
            arr_h_time_without_charge[i] = v_h_time_without_charge[i];
            arr_h_time_with_charge[i]    = v_h_time_with_charge[i];
        }

        // Initialize DataStore for PSDSklearn.py so that we can get PSD results from Python Alg. ( For some initialization order reasons, we cannot set PyDataStore in PSDTool_sklearn::initialize())
        if (evtID==0)
        {
            SniperDataPtr<PyDataStore>pystore(sub_task, "DataStore");

            if (pystore.invalid())
                LogError << "Failed to find the PyDataStore. Register the value to module " << std::endl;

            m_ds = pystore.data();
            m_ds->set("PSDVal", 0);
        }

        // register variables for python
        LogInfo << "Register the value to PyDataStore. " << std::endl;
        m_ds->set("h_time_with_charge", arr_h_time_with_charge);
        m_ds->set("h_time_without_charge", arr_h_time_without_charge);
        m_ds->set("xyz_E", arr_xyz_and_E);
        m_ds->set("path_model", path_model);
//        m_ds->set("PSDInput", v_PSDInput);
    }

    return true;
}

double PSDTool_sklearn::CalPSDVal(){
    // Run PyAlg PSDSklearn.py by fire sub_task ( sincerely thanks for Tao Lin and Yuxiang Hu 's help! )
    Incident::fire(*getParent(), iotaskname);

    // Get PSDVal results from PSDSklearn.py
    m_ds->get("PSDVal",m_psdEvent.psdVal);
    LogInfo<< "PSDVal-->"<< m_psdEvent.psdVal << endl;

    // Set default tag for prediction output
    if (m_psdEvent.psdVal >= PSD_divide)
        m_psdEvent.evtType = EVTTYPE::Electron;
    else
        m_psdEvent.evtType = EVTTYPE::Proton;

    // Save PSD results
    m_psdTree->Fill();

    return m_psdEvent.psdVal;
}

bool isSpace(char x) {return x== ' ';}
void PSDTool_sklearn::SetBins()
{
    auto ss = ostringstream{};
    ifstream bins_file(m_path_bins_file);
    if(!bins_file.is_open())
    {
        cerr<< "Could not open file ("<<m_path_bins_file<<") for bins setting!!"<<endl;
        exit(EXIT_FAILURE);
    }
    ss << bins_file.rdbuf();
    istringstream  sstream(ss.str());
    string record;
    char delimiter = ',';

    while (std::getline(sstream, record))
    {
        istringstream line(record);
        while(getline(line, record, delimiter))
        {
            record.erase(std::remove_if(record.begin(), record.end(), isSpace), record.end());
            m_bins.push_back(stod(record));
        }
    }

}

