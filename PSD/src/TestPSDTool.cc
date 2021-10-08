#include "TestPSDTool.h"

#include "SniperKernel/ToolBase.h"
#include "SniperKernel/ToolFactory.h"
#include "SniperKernel/SniperPtr.h"
#include "RootWriter/RootWriter.h"

DECLARE_TOOL(TestPSDTool);

TestPSDTool::TestPSDTool(const std::string &name): ToolBase(name){
    d_psdVar = 0;
}

TestPSDTool::~TestPSDTool(){
}

bool TestPSDTool::initialize(){
    LogInfo<<"Initializing TestPSDTool..."<<std::endl;
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

    //Store the PSD result, which may be implemented in data model in the future
    m_psdTree=rwsvc->bookTree("PSD", "PSD");
    m_psdTree->Branch("psdVal", &m_psdEvent.psdVal, "psdVal/D");
    m_psdTree->Branch("evtType", &m_psdEvent.evtType, "psdVal/I");
    rwsvc->attach("PSD_OUTPUT", m_psdTree);

    m_psdInput = new PSDInput();
    return true;
}

bool TestPSDTool::finalize(){
    LogInfo<<"Finalizing TestPSDTool..."<<std::endl;
    delete m_psdInput;
    return true;
}

bool TestPSDTool::preProcess(JM::CalibEvent *calibevent, JM::CDRecEvent *recevent){
    LogDebug<<"pre processing an event..."<<std::endl;
    if (!m_psdInput->extractHitInfo(calibevent, recevent)) return false;
    d_psdVar = 100;
    if (!b_usePredict) m_userTree->Fill();
    return true;
}

double TestPSDTool::predict(){
    m_psdEvent.psdVal = 0.3;
    m_psdEvent.evtType = EVTTYPE::Electron;
    m_psdTree->Fill();
    return m_psdEvent.psdVal;
}
