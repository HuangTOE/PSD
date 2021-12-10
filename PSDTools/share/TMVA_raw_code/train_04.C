#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"
#include "TString.h"
#include "TObjString.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"
#include "TMVA/TMVAGui.h"

using namespace TMVA;

void train_04(int vol, int charge, int isCut, int ver)
{
  // TFile *Output1 = new TFile("/junofs/users/chengjie/workdir/PSDTools/Ana-new0817/TMVA/preInput/train_atm1026mt01fs01.root");
  //
  // TString volname[6] = {"v0", "v1", "v2", "v3", "v4", "v5"};
  TString volname[2] = {"v0", "v5"};
  TString charname[2] = {"wocharge", "wcharge"};
  TString ver01[2] = {"J20", "J21"};
  TString ver02[2] = {"allJ20", "allJ21"};
  TString inputdir = "/junofs/users/chengjie/workdir/PSDTools/Ana-new0817/TMVA-20210421/input/" + ver01[ver] + "/forTMVA";

  TString sigfname = "";
  TString bkgfname = "";
  if (isCut == 1) {
    sigfname = inputdir + "/dsnb_" + charname[charge] + "_" + volname[vol] + ".root";
    bkgfname = inputdir + "/atm_" + charname[charge] + "_" + volname[vol] + ".root";
  } else {
    //sigfname = inputdir + "/dsnb_" + charname[charge] + "_" + volname[vol] + ver02[ver] + ".root";
    //bkgfname = inputdir + "/atm_" + charname[charge] + "_" + volname[vol] + ver02[ver] + ".root";
    sigfname = inputdir + "/dsnb_" + charname[charge] + "_" + volname[vol] + ver02[ver] + "_chargeupdated.root";
    bkgfname = inputdir + "/atm_" + charname[charge] + "_" + volname[vol] + ver02[ver] + "_chargeupdated.root";
  }
  // TString bkgfname = inputdir + "/fn_" + charname[charge] + "_" + volname[vol] + ".root";

  cout << sigfname << endl;
  cout << bkgfname << endl;

  TFile *Output1 = new TFile(bkgfname);
  TFile *Output2 = new TFile(sigfname);
  TTree *tSig = (TTree *)Output2->Get("sigtree");
  TTree *tBkg = (TTree *)Output1->Get("bkgtree");

  TString outdir = "/junofs/users/chengjie/workdir/PSDTools/Ana-new0817/TMVA-20210421/output";

  TString outfileName = "";
  if (isCut == 1) {
    outfileName = outdir + "/tmva_" + charname[charge] + "_" + volname[vol] + ver01[ver] + "_wcut_wor3.root";
  } else {
    outfileName = outdir + "/tmva_" + charname[charge] + "_" + volname[vol] + ver01[ver] + "_wocut_wor3.root";
  }
  TFile *outputFile = TFile::Open(outfileName, "RECREATE");
  std::string fWeightOutput = "train1.root";
  double SigWeight = 1.0;
  double BkgWeight = 1.0;
  TMVA::Factory *factory = new TMVA::Factory("Induction_", outputFile, "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification");
  // TMVA::Factory *factory = new TMVA::Factory( "Induction_"+fWeightOutput, Output,"!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
  TMVA::DataLoader *dataloader = new TMVA::DataLoader("bkgRejection");
  dataloader->AddBackgroundTree(tBkg, BkgWeight);
  dataloader->AddSignalTree(tSig, SigWeight);
  dataloader->AddVariable("w1", 'F');
  dataloader->AddVariable("w2", 'F');
  dataloader->AddVariable("tau2", 'F');
  dataloader->AddVariable("tau3", 'F');
  dataloader->AddVariable("eta", 'F');
  // dataloader->AddVariable("vtag", 'I');
  dataloader->AddSpectator("Eqe_tag", "Eqe_tag", "units", 'F');
  dataloader->AddSpectator("nPE_tag", "nPE_tag", "units", 'F');
  dataloader->AddSpectator("r3_tag", "r3_tag", "units", 'F');
  dataloader->AddSpectator("id_tag", "id_tag", "units", 'F');
  dataloader->AddSpectator("isoz", "isoz", "units", 'I');
  dataloader->AddSpectator("ison", "ison", "units", 'I');

  // dataloader->SetBackgroundWeightExpression( "weight" );

  TCut mycuts = "";
  TCut mycutb = "";
  int ntrain[6] = {180000, 45000, 44000, 44000, 48000, 44000};
  dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=ntrain[vol]:nTrain_Background=ntrain[vol]:nTest_Signal=100:nTest_Background=100:SplitMode=Random:NormMode=NumEvents:!V");

  // int ntrain[6] = {180000, 45000, 44000, 44000, 48000, 44000};
  factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDTG", "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2");
  // factory->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=1000:nTrain_Background=1000:SplitMode=Random:NormMode=NumEvents:!V");
  // factory->PrepareTrainingAndTestTree(mycuts, ntrain[vol], ntrain[vol], 100, 100, "");
  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();
  Output1->Close();
  Output2->Close();
  outputFile->Close();
  std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
  std::cout << "==> TMVAClassification is done!" << std::endl;
  delete factory;
  delete dataloader;
  // if (!gROOT->IsBatch()) TMVA::TMVAGui(outfileName);
  // return 0;
}
