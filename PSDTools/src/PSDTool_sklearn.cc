//
// Created by luoxj@ihep.ac.cn on 2021/12/19.
//

#include "../PSDTools/PSDTool_sklearn.h"

#include "SniperKernel/ToolBase.h"
#include "SniperKernel/ToolFactory.h"
#include "SniperKernel/SniperPtr.h"
#include "SniperKernel/SniperDataPtr.h"

#include "RootWriter/RootWriter.h"


#include "SniperPython/PyDataStore.h"

#include "EvtNavigator/NavBuffer.h"

namespace p = boost::python;
namespace np = boost::python::numpy;

using namespace std;
DECLARE_TOOL(PSDTool_sklearn);

PSDTool_sklearn::PSDTool_sklearn(const std::string &name): ToolBase(name){
    d_psdVar = 0;
}

PSDTool_sklearn::~PSDTool_sklearn(){
}

bool PSDTool_sklearn::initialize(){
    LogInfo<<"Initializing PSDTool_sklearn..."<<std::endl;
    //get the rootwriter
    SniperPtr<RootWriter> rwsvc(getParent(),"RootWriter");
    if ( rwsvc.invalid() ) {
        LogError << "cannot get the rootwriter service." << std::endl;
        return false;
    }

    //Store the pre-processed events
    m_userTree=rwsvc->bookTree("evt", "evt");
    m_userTree->Branch("PSDVar", &d_psdVar, "PSDVar/D");
    rwsvc->attach("USER_OUTPUT", m_userTree);

    //Store the PSDTools result, which may be implemented in data model in the future
    m_psdTree=rwsvc->bookTree("PSDTools", "PSDTools");
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
    if (!m_psdInput->extractHitInfo(nav,"alignPeak2")) return false;
    const vector<double> v_hittime = m_psdInput->getHitTime();
    const vector<double> v_charge = m_psdInput->getHitCharge();

    // Set python array
    p::tuple shape = p::make_tuple(100);
    np::dtype dtype_int = np::dtype::get_builtin<int>();
    np::dtype dtype_double = np::dtype::get_builtin<double>();

    np::ndarray arr_hist = np::zeros(shape, dtype_double);
    arr_hist[1] = 100;
    arr_hist[5] = 500;

    // register variables for python

    SniperDataPtr<PyDataStore> pystore(*getRoot(), "DataStore");
    if (pystore.invalid()) {
        LogError << "Failed to find the PyDataStore. Register the value to module " << std::endl;

        p::object this_module = p::import("ExamplePyAlg");
        this_module.attr("h_time") = arr_hist;
    } else {
        LogInfo << "Register the value to PyDataStore. " << std::endl;
        pystore->set("h_time", arr_hist);
    }


    d_psdVar = 100;
    if (!b_usePredict) m_userTree->Fill();
    return true;
}

double PSDTool_sklearn::CalPSDVal(){
    m_psdEvent.psdVal = 0.3;
    m_psdEvent.evtType = EVTTYPE::Electron;
    m_psdTree->Fill();
    return m_psdEvent.psdVal;
}


