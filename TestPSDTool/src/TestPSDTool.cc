#include "TestPSDTool.h"

#include "SniperKernel/ToolBase.h"
#include "SniperKernel/ToolFactory.h"
#include "SniperKernel/SniperPtr.h"
#include "RootWriter/RootWriter.h"

using namespace std;
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

bool TestPSDTool::preProcess(JM::EvtNavigator *nav){
    LogDebug<<"pre processing an event..."<<std::endl;
    if (!m_psdInput->extractHitInfo(nav)) return false;

    // Extract  Raw Waveform from ElecSim
    if (!m_psdInput->extractHitsWaveform(nav)) return false;
    vector<vector<unsigned int >> v2d_waveforms = m_psdInput->getHitsWaveform();
    //for(int i=0;i<v2d_waveforms[1].size();i++) cout << " " <<v2d_waveforms[0][i]<< " ";
    //cout<<endl;
    
    d_psdVar = 100;
    if (!b_usePredict) m_userTree->Fill();
    return true;
}

double TestPSDTool::CalPSDVal(){
    m_psdEvent.psdVal = 0.3;
    m_psdEvent.evtType = EVTTYPE::Electron;
    m_psdTree->Fill();
    return m_psdEvent.psdVal;
}
