#include "TMVATrainAlg.h"
#include "SniperKernel/AlgFactory.h"
#include "SniperKernel/SniperPtr.h"
#include "SniperKernel/SniperDataPtr.h"
#include "BufferMemMgr/EvtDataPtr.h"
#include "Event/PidTmvaHeader.h"
#include "Event/SimHeader.h"
#include "Event/ElecHeader.h"
#include "BufferMemMgr/IDataMemMgr.h"

#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"

#include "TMVA/DataLoader.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"


DECLARE_ALGORITHM(TMVATrainAlg);

	TMVATrainAlg::TMVATrainAlg(const std::string& name)
: AlgBase(name)
{
	declProp("SigInput",	fSigInput="Sig"); // signal input
	declProp("BkgInput",	fBkgInput="Bkg"); // background input
	declProp("Output",	fOutput="train.root"); // background input
	declProp("WeightOutput",	fWeightOutput="train"); // background input

}

TMVATrainAlg::~TMVATrainAlg()
{

}

	bool
TMVATrainAlg::initialize()
{
	fSigTree="SigTree";
	fBkgTree="BkgTree";
	Nsig=0;
	Nbkg=0;
	tSig = new TTree(fSigTree.c_str(),"SigTree");
	tBkg = new TTree(fBkgTree.c_str(),"BkgTree");
	tSig->Branch("dx_max",			&sig_dx_max,			"dx_max/D");
	tSig->Branch("dy_max",			&sig_dy_max,			"dy_max/D");
	tSig->Branch("charge",			&sig_charge,			"charge/D");
	tSig->Branch("rising_time",	&sig_rising_time,		"rising_time/D");
	tSig->Branch("channel_num",	&sig_channel_num,		"channel_num/D");
	tSig->Branch("peak_num",		&sig_peak_num,			"peak_num/D");
	tBkg->Branch("dx_max",			&bkg_dx_max,			"dx_max/D");
	tBkg->Branch("dy_max",			&bkg_dy_max,			"dy_max/D");
	tBkg->Branch("charge",			&bkg_charge,			"charge/D");
	tBkg->Branch("rising_time",	&bkg_rising_time,		"rising_time/D");
	tBkg->Branch("channel_num",	&bkg_channel_num,		"channel_num/D");
	tBkg->Branch("peak_num",		&bkg_peak_num,			"peak_num/D");
	EventNum=0;

	return true;
}

	bool
TMVATrainAlg::execute()
{
	EventNum++;
	std::cout<<EventNum<<std::endl;
	EvtDataPtr<nEXO::PidTmvaHeader> header(getParent(), "/Event/PidTmva");
	if (header.invalid()) {
		LogError << "can't find the header." << std::endl;
		return false;
	}
	EvtDataPtr<nEXO::ElecHeader> Elecheader(getParent(), "/Event/Elec");
	nEXO::ElecEvent* Elecevent = dynamic_cast<nEXO::ElecEvent*>(Elecheader->event());

	//signal input
	
	if((header->get_type()=="bb0n"||header->get_type()=="elec")&&header->get_charge()>80000&&Nsig<25000&&header->get_channel_num()>0&&header->get_rising_time()>0){//&&header->get_outside()==false){
		sig_dx_max = header->get_dx_max();
		sig_dy_max = header->get_dy_max();
		sig_charge = header->get_charge();
		sig_rising_time = header->get_rising_time();
		sig_channel_num = header->get_channel_num();
		sig_peak_num = header->get_peak_num();
		Nsig++;
		tSig->Fill();
	}
	//background input
	else if(header->get_type()=="gamma"&&header->get_charge()>80000&&Nbkg<25000&&header->get_channel_num()>0&&header->get_rising_time()>0){//&&header->get_outside()==false){
		bkg_dx_max = header->get_dx_max();
		bkg_dy_max = header->get_dy_max();
		bkg_charge = header->get_charge();
		bkg_rising_time = header->get_rising_time();
		bkg_channel_num = header->get_channel_num();
		bkg_peak_num = header->get_peak_num();
		Nbkg++;
		tBkg->Fill();
	}

	return true;
}

	bool
TMVATrainAlg::finalize()
{
	std::cout<<std::endl<<Nsig<<"	"<<Nbkg<<std::endl;
	Output = TFile::Open( fOutput.c_str(), "RECREATE" );
	tSig->Write();
	tBkg->Write();
	double SigWeight = 1.0;
	double BkgWeight = 1.0;
	TMVA::DataLoader *dataloader = new TMVA::DataLoader("GammaRejection");
	TMVA::Factory *factory = new TMVA::Factory( "Induction_"+fWeightOutput, Output,"!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
	dataloader->AddBackgroundTree(tBkg,BkgWeight);
	dataloader->AddSignalTree(tSig,SigWeight);
	dataloader->AddVariable("channel_num", 'F');
	dataloader->AddVariable("dx_max",  'F');
	dataloader->AddVariable("dy_max",  'F');
	dataloader->AddVariable("rising_time",  'F');
	dataloader->AddVariable("peak_num", 'F');
	TCut mycuts = "";
	TCut mycutb = "";
	factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDTG","!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2" );
//	factory->PrepareTrainingAndTestTree("Charge>80000", "nTrain_Signal=7000:nTrain_Background=7000:nTest_Signal=3000:nTest_Background=3000");
	factory->TrainAllMethods();
	factory->TestAllMethods();
	factory->EvaluateAllMethods();
	Output->Close();
	delete factory;

	return true;
}
