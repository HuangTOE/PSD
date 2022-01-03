#include "PSDTools/PSDAlg.h"

#include "SniperKernel/ToolBase.h"
#include "SniperKernel/AlgFactory.h"
#include "RootWriter/RootWriter.h"
#include "Event/RecHeader.h"
#include "Geometry/PMTParamSvc.h"
#include "PSDTools/IPSDInputSvc.h"


DECLARE_ALGORITHM(PSDAlg);

PSDAlg::PSDAlg(const std::string &name):AlgBase(name){
    i_ithEvt = -1;
    m_psdTool = nullptr;

    d_psdVal = -1;
    d_recE = 0;
    d_recX = 0;d_recY = 0;d_recZ = 0;

    declProp("Method", s_psdMethod = "");
    declProp("UsePredict", b_usePredict = true);
    declProp("Model", b_model="");
}

PSDAlg::~PSDAlg()= default;

bool PSDAlg::initialize(){
    //==============Get the event buffer==============
    SniperDataPtr<JM::NavBuffer>  navBuf(getParent(), "/Event");
    if ( navBuf.invalid() ) {
        LogError << "cannot get the NavBuffer @ /Event" << std::endl;
        return false;
    }
    m_buf = navBuf.data();


    //==============Initialize the PSDTools method==============
//    SniperDataPtr<IPMTParamSvc> pmt_svc(getParent(), "PMTParamSvc");
//    if (pmt_svc.invalid())
//    {
//        LogError<<"The PMTParamSvc is not found!"<<std::endl;
//        return false;
//    }

    IPSDInputSvc* psdInputSvc = dynamic_cast<IPSDInputSvc*>(getParent()->find("PSDInputSvc"));
    psdInputSvc->Initialize(dynamic_cast<PMTParamSvc *>(getParent()->find("PMTParamSvc")));

    m_psdTool=tool<IPSDTool>(s_psdMethod);
    if (!m_psdTool){
        LogError<<"The PSDTools tool is not found!"<<std::endl;
        return false;
    }
    if (b_usePredict) m_psdTool->enablePredict();
    else m_psdTool->disablePredict();

//    m_psdTool->setModelToPredict(b_model);

    if (!m_psdTool->initialize()){
        LogError<<"Fail to initialize Tool:"<<s_psdMethod<<"!"<<std::endl;
        return false;
    }

    //==============initialize the output file===================
    SniperPtr<RootWriter> rwsvc(getParent(),"RootWriter");
    if ( rwsvc.invalid() ) {
        LogError << "cannot get the rootwriter service." << std::endl;
        return false;
    }
    m_outTree=rwsvc->bookTree("PSDUser", "PSDUser");
    m_outTree->Branch("evtID", &i_ithEvt, "evtID/I");
    m_outTree->Branch("recE", &d_recE, "recE/D");
    m_outTree->Branch("recX", &d_recX, "recX/D");
    m_outTree->Branch("recY", &d_recY, "recY/D");
    m_outTree->Branch("recZ", &d_recZ, "recZ/D");
    rwsvc->attach("USER_OUTPUT", m_outTree);
    return true;
}

bool PSDAlg::finalize(){
    if (!m_psdTool->finalize()){
        LogError<<"Fail to finalize Tool:"<<s_psdMethod<<"!"<<std::endl;
        return false;
    }
    return true;
}

bool PSDAlg::execute(){
    i_ithEvt++;
    LogInfo<<i_ithEvt<<"th event!"<<std::endl;

    //===============get the current event=================
    JM::EvtNavigator *nav=m_buf->curEvt();


    //=====================PSDTools procedure=====================
    if (!m_psdTool->preProcess(nav)){
        LogError<<"Error when pre-processing the "<<i_ithEvt<<"th event!"<<std::endl;
        return false;
    }
    if (b_usePredict) d_psdVal = m_psdTool->CalPSDVal();

    // ==============Get current rec event ==========================================
    JM::RecHeader *recheader=dynamic_cast<JM::RecHeader*>(nav->getHeader("/Event/Rec"));
    JM::CDRecEvent *thisRecEvent=recheader->cdEvent();

    //===============get the reconstruction information=================
    d_recE=thisRecEvent->energy();
    d_recX=thisRecEvent->x();
    d_recY=thisRecEvent->y();
    d_recZ=thisRecEvent->z();
    LogInfo<<"Energy:"<<d_recE<<"MeV"<<std::endl;
    LogInfo<<"Vertex: ("<<d_recX<<", "<<d_recY<<", "<<d_recZ<<") mm."<<std::endl;
    m_outTree->Fill();

    return true;
}
