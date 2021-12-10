#include "SNMLTool.h"

#include "SniperKernel/ToolFactory.h"
#include "SniperKernel/SniperPtr.h"
#include "RootWriter/RootWriter.h"

DECLARE_TOOL(SNMLTool);

SNMLTool::SNMLTool(const std::string &name): ToolBase(name){
    i_ithEvt = -1;
    declProp("truthFile", s_truthFileName="");
    declProp("weightFile", s_weightFileName="");
}

SNMLTool::~SNMLTool(){
}

bool SNMLTool::init(){
    LogInfo<<"Initializing SNMLTool..."<<std::endl;
    //get the rootwriter
    SniperPtr<RootWriter> rwsvc(getParent(),"RootWriter");
    if ( rwsvc.invalid() ) {
        LogError << "cannot get the rootwriter service." << std::endl;
        return false;
    }

    //for extracting the hit time information of a specfic event
    m_psdInput = new PSDInput();
    i_nBins=550;
    d_lowEdge=-100;
    d_highEdge=1000;
    m_hitTimeForEvt=new TH1D("thisHist", "ML Variable", i_nBins, d_lowEdge, d_highEdge);
    d_psdVar = new double[i_nBins];
    for (int i=0;i<i_nBins;i++){
        d_psdVar[i] = 0;
    }
    d_recE=-1;
    s_truthEvtType = "";

    if (b_usePredict){
        //Store the PSDTools result, which may be implemented in data model in the future
        m_psdTree=rwsvc->bookTree("PSDTools", "PSDTools");
        m_psdTree->Branch("psdVal", &m_psdEvent.psdVal, "psdVal/D");
        m_psdTree->Branch("evtType", &m_psdEvent.evtType, "evtType/I");
        rwsvc->attach("PSD_OUTPUT", m_psdTree);
        initForPredict();
    }
    else{
        //Store the pre-processed events
        m_userTree=rwsvc->bookTree("evt", "evt");
        m_userTree->Branch("truthType", &s_truthEvtType);
        m_userTree->Branch("recE", &d_recE, "recE/D");
        for (int i=0;i<i_nBins;i++){
            m_userTree->Branch(Form("Var%d", i), &d_psdVar[i], Form("Var%d/D", i));
        }
        rwsvc->attach("USER_OUTPUT", m_userTree);
        //m_userTree.clear();
        //for (int ith=0;ith<i_NEvtTypes;ith++){
        //    std::string evtname=s_evtType[ith];
        //    m_userTree[evtname]=rwsvc->bookTree(evtname.c_str(), evtname.c_str());
        //    m_userTree[evtname]->Branch("recE", &d_recE, "recE/D");
        //    for (int i=0;i<i_nBins;i++){
        //        m_userTree[evtname]->Branch(Form("Var%d", i), &d_psdVar[i], Form("Var%d/D", i));
        //    }
        //    rwsvc->attach("USER_OUTPUT", m_userTree[evtname]);
        //}
    }

    return true;
}

bool SNMLTool::initForPredict(){
    m_weightFile = TFile::Open(s_weightFileName.c_str(), "READ");
    m_psdValTree = dynamic_cast<TTree*>(m_weightFile->Get("PSDVal"));//tree stores the predict value
    m_psdValTree->SetBranchAddress("psdval", &d_predictVal);
    return true;
}

bool SNMLTool::fina(){
    LogInfo<<"Finalizing SNMLTool..."<<std::endl;
    delete m_psdInput;
    //delete m_hitTimeForEvt;
    delete[] d_psdVar;
    if (b_usePredict) m_weightFile->Close();
    return true;
}

bool SNMLTool::preProcess(JM::CalibEvent* calibevent, JM::CDRecEvent* recevent){
    i_ithEvt++;

    //reconstructed energy
    d_recE = recevent->energy();

    if (!m_psdInput->extractHitInfo(calibevent, recevent)) return false;
    m_psdInput->alignTime("alignPeak");
    std::vector<double> hittime = m_psdInput->getHitTime();
    m_hitTimeForEvt->Reset();
    for (std::vector<double>::iterator it=hittime.begin();it!=hittime.end();++it){
        m_hitTimeForEvt->Fill(*it);
    }

    //Hit time PDF
    for (int ith=0;ith<i_nBins;ith++){
        d_psdVar[ith]=m_hitTimeForEvt->GetBinContent(ith+1);
    }

    if (!b_usePredict){
        //std::string thistruthtype = getTruthEvtType(i_ithEvt);
        //m_userTree[thistruthtype]->Fill();
        s_truthEvtType = getTruthEvtType(i_ithEvt);
        m_userTree->Fill();
    }
    return true;
}

double SNMLTool::predict(){
    //this is fake, predict is done by python script
    //just reading the result of predict by python
    m_psdValTree->GetEntry(i_ithEvt);
    m_psdEvent.psdVal = d_predictVal;
    if (m_psdEvent.psdVal>0.5) m_psdEvent.evtType = EVTTYPE::Proton;
    else m_psdEvent.evtType = EVTTYPE::Electron;
    m_psdTree->Fill();
    return m_psdEvent.psdVal;
}
