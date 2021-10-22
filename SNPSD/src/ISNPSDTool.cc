#include "ISNPSDTool.h"

#include "SniperKernel/SniperPtr.h"
#include "SniperKernel/SniperLog.h"
#include "RootWriter/RootWriter.h"

ISNPSDTool::ISNPSDTool(){
    m_truthFile = NULL;
    m_truthTree = NULL;
}

ISNPSDTool::~ISNPSDTool(){
}

bool ISNPSDTool::initialize(){
    if (!b_usePredict){
        if (!s_truthFileName.compare("")){
            LogError<<"The truth file is not specified!"<<std::endl;
            return false;
        }
        //open the truth file for event type
        m_truthFile = TFile::Open(s_truthFileName.c_str(), "READ");
        m_truthTree=dynamic_cast<TTree*>(m_truthFile->Get("SNTruth"));
        m_truthTree->SetBranchAddress("evtType", &s_truthEvtType);
    }

    return init();
}

bool ISNPSDTool::finalize(){
    if (!b_usePredict){
        m_truthFile->Close();
    }
    return fina();
}

std::string ISNPSDTool::getTruthEvtType(int evtid){
    m_truthTree->GetEntry(evtid);
    return *s_truthEvtType;
}
