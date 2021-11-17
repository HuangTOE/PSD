#include "TMVAReaderAlg.h"
#include "SniperKernel/AlgFactory.h"
#include "SniperKernel/AlgBase.h"
#include "SniperKernel/SniperPtr.h"
#include "SniperKernel/SniperDataPtr.h"
#include "BufferMemMgr/EvtDataPtr.h"
#include "Event/PidTmvaHeader.h"
#include "Event/SimHeader.h"
#include "Event/ElecHeader.h"
#include "Event/TMVATag.h"
#include "BufferMemMgr/IDataMemMgr.h"

#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"
#include "TH1.h"

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"

DECLARE_ALGORITHM(TMVAReaderAlg);

	TMVAReaderAlg::TMVAReaderAlg(const std::string& name)
: AlgBase(name)
{
	declProp("InputTrain", filename="/dybfs2/nEXO/fuys/BDT-analysis/gamma-bb0n-train-jobs/chargeCut-BDTG-tmva-bb0n-gamma-jobs/training3/train-2350-bb0n.root");
	declProp("WeightName", weightname="train");
	declProp("WeightDirBDTG", weightdir_bdtg="/dybfs2/nEXO/fuys/BDT-analysis/gamma-bb0n-train-jobs/chargeCut-BDTG-tmva-bb0n-gamma-jobs/training3/GammaRejection/weights");
	declProp("WeightDirBDT", weightdir_bdt="/dybfs2/nEXO/fuys/BDT-analysis/gamma-bb0n-train-jobs/BDT-tmva-bb0n-gamma-jobs/training-3/GammaRejection/weights");
}

TMVAReaderAlg::~TMVAReaderAlg()
{

}

	bool
TMVAReaderAlg::initialize()
{
    std::string Method_str_BDTG = weightdir_bdtg+"/Induction_"+weightname+"_BDTG.weights.xml";
	TMVA::Tools::Instance();
	std::string ReaderCoef_BDTG="!Color:!Silent";
	reader_BDTG  = new TMVA::Reader( ReaderCoef_BDTG.c_str() );
	reader_BDTG->AddVariable("channel_num", &channel_num);
	reader_BDTG->AddVariable("dx_max", 				&dx_max);
	reader_BDTG->AddVariable("dy_max",  			&dy_max);
	reader_BDTG->AddVariable("rising_time",			&rising_time);
	reader_BDTG->AddVariable("peak_num",			&peak_num);
	reader_BDTG->BookMVA("BDTG method",Method_str_BDTG);

    std::string Method_str_BDT = weightdir_bdt+"/Induction_"+weightname+"_BDT.weights.xml";
	TMVA::Tools::Instance();
	std::string ReaderCoef_BDT="!Color:!Silent";
	reader_BDT  = new TMVA::Reader( ReaderCoef_BDT.c_str() );
	reader_BDT->AddVariable("channel_num", &channel_num);
	reader_BDT->AddVariable("dx_max", 				&dx_max);
	reader_BDT->AddVariable("dy_max",  				&dy_max);
	reader_BDT->AddVariable("rising_time",			&rising_time);
	reader_BDT->AddVariable("peak_num",				&peak_num);
	reader_BDT->BookMVA("BDT method",Method_str_BDT);
	
    return true;
}

	bool
TMVAReaderAlg::execute()
{
    EvtDataPtr<nEXO::PidTmvaHeader> header(getParent(), "/Event/PidTmva");
	if (header.invalid()) {
		LogError << "can't find the header." << std::endl;
		return false;
	}
	
        dx_max = (float)(header->get_dx_max());
		dy_max = (float)(header->get_dy_max());
		rising_time = (float)(header->get_rising_time());
		channel_num = (float)(header->get_channel_num());
		peak_num = (float)(header->get_peak_num());
	val_BDTG = (float)(reader_BDTG->EvaluateMVA("BDTG method"));
   	val_BDT = (float)(reader_BDT->EvaluateMVA("BDT method"));
    
    SniperDataPtr<nEXO::NavBuffer>	navBuf(m_par, "/Event");
    if (navBuf.invalid()) {
        LogError << "Can't find the buffer " << std::endl;
        return false;
    }
	nEXO::EvtNavigator* nav = navBuf->curEvt();
    if (!nav) {
        LogError << "Can't find the event navigator " << std::endl;
        return false;
    }
    /////saver^
	nEXO::TMVATagHeader* tmva_header = new nEXO::TMVATagHeader();
	nav->addHeader(tmva_header);
	
    nEXO::TMVATag* tmva_event = new nEXO::TMVATag();
    tmva_header->setEvent(tmva_event);
    tmva_event->m_TMVA_Tag_BDTG = val_BDTG;
    tmva_event->m_TMVA_Tag_BDT = val_BDT;
    

	return true;

}
	bool
TMVAReaderAlg::finalize()
{
	return true;
}
