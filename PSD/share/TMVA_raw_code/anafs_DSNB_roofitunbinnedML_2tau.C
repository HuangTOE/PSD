#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TString.h>
#include <TRandom.h>
#include <TF1.h>
#include <TStyle.h>
#include <stdio.h>
#include <TProfile.h>
#include <TSystem.h>
#include <TROOT.h>
#include <algorithm>

#include <iomanip>
#include <map>

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooDataSet.h"
#include "RooConstVar.h"
#include "RooFormulaVar.h"
#include "RooExponential.h"
#include "RooPolynomial.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooFitResult.h"

using namespace std;
using namespace RooFit;

TString outputFile = "a.root";

int evtID_n;
int stepNumber;
int PDG[10000];
double Positionx[10000];
double Positiony[10000];
double Positionz[10000];
double GlobalTime[10000];
double TotalEnergy[10000];
double KineticEnergy[10000];
double EnergyDeposit[10000];
double QEnergyDeposit[10000];
double Captime[10000];

int evtID, nPhotons, totalPE;
float edep;
float edepX, edepY, edepZ;

int gevtID, initpdg;
float initx, inity, initz, initpx, initpy, initpz;
double inittime;

double Pi = 3.1415926;
const int nPMTs = 17612;
double PMTpos_x[nPMTs];
double PMTpos_y[nPMTs];
double PMTpos_z[nPMTs];

const int nPMTs2 = 17612;
double darkRate[nPMTs2];
double timeSpread[nPMTs2];
double timeOffset[nPMTs2];
int isHamamatsu[nPMTs2];
double ndarkrate0;
double ndarkrate1;
double ndarkrate2;

float recx;
float recy;
float recz;

struct Event {
  int id_evt;
  int pdg_evt;
  double x_evt;
  double y_evt;
  double z_evt;
  double edep_evt;
  double t_evt;
  double qe_evt;
};

struct eleEvent {
  int sim_evt;
  int ele_evt;
  int status_evt;
  double trigTime_evt;
};

struct fitRes {
  double tau1;
  double tau1err;
  double tau2;
  double tau2err;
  double tau1ratio;
  double tau1ratioerr;
  double ndark;
  double nn0;
  double chi2;
  double ndf;
};
void PMTPosition();
void ReadPMTdata();
double GetTimetag(TH1F* h1tem);
double GetPeakvalue(TH1F* h1tem);
double GetTail2total(TH1F* h1tem, double ttag, int istep);
double GetW1new(TH1F* h1tem, double cut);
double GetW2new(TH1F* h1tem, double cut);

double GetW1(TH1F* h1tem, int ss);
double GetW2(TH1F* h1tem, int ss);
double GetNdf(TTree* ttree, int isWeight, double xlow, double xhigh);

struct fitRes myroofit(TTree* ttree, TH1F* h2, int evttag, int fittag, int isWeight, double xhigh);

typedef pair<int, double> PAIR;
int cmp(const PAIR& x, const PAIR& y)  //
{
  return x.second > y.second;
}

typedef pair<double, double> PAIR02;
int cmp02(const PAIR02& x, const PAIR02& y)  //
{
  return x.first < y.first;
}

void anafs_DSNB_roofitunbinnedML_2tau(int fileNo)
{

  cout.precision(10);
  cout.setf(ios::showpoint);
  gStyle->SetOptFit(1111);
  TH1::SetDefaultSumw2();

  TString ID = "";  //---J21
  ID += fileNo;     //---J21

  TString datapath = "/dybfs/users/chengjie/data/PSD/J21/DSNB/";  //---J21
  TString datapath1 = "/dybfs/users/chengjie/data/PSD/J21/DSNB";  //--- J21
  TString fileName1 = datapath1 + "/DS/user_" + ID + ".root";     //---J21
  cout << "===> detsim file : " << fileName1 << endl;

  TFile* f = TFile::Open(fileName1);
  int nentries;

  TTree* depT = (TTree*)f->Get("depTree");
  depT->SetBranchAddress("evtID", &evtID_n);
  depT->SetBranchAddress("stepNumber", &stepNumber);
  depT->SetBranchAddress("PDG", PDG);
  depT->SetBranchAddress("Positionx", Positionx);
  depT->SetBranchAddress("Positiony", Positiony);
  depT->SetBranchAddress("Positionz", Positionz);
  depT->SetBranchAddress("GlobalTime", GlobalTime);
  depT->SetBranchAddress("TotalEnergy", TotalEnergy);
  depT->SetBranchAddress("EnergyDeposit", EnergyDeposit);
  depT->SetBranchAddress("QEnergyDeposit", QEnergyDeposit);
  depT->SetBranchAddress("Captime", Captime);

  TTree* evt = (TTree*)f->Get("evt");
  evt->SetBranchAddress("evtID", &evtID);
  evt->SetBranchAddress("nPhotons", &nPhotons);
  evt->SetBranchAddress("totalPE", &totalPE);
  evt->SetBranchAddress("edep", &edep);
  evt->SetBranchAddress("edepX", &edepX);
  evt->SetBranchAddress("edepY", &edepY);
  evt->SetBranchAddress("edepZ", &edepZ);

  TTree* geninfo = (TTree*)f->Get("geninfo");
  geninfo->SetBranchAddress("evtID", &gevtID);
  geninfo->SetBranchAddress("InitPDGID", &initpdg);
  geninfo->SetBranchAddress("InitX", &initx);
  geninfo->SetBranchAddress("InitY", &inity);
  geninfo->SetBranchAddress("InitZ", &initz);
  geninfo->SetBranchAddress("InitPX", &initpx);
  geninfo->SetBranchAddress("InitPY", &initpy);
  geninfo->SetBranchAddress("InitPZ", &initpz);
  geninfo->SetBranchAddress("InitTime", &inittime);

  int ne1 = evt->GetEntries();
  int ne2 = geninfo->GetEntries();

  TString fileName2 = datapath + "/CS/calib_user_" + ID + ".root";  //--J21
  TFile* calibf1 = TFile::Open(fileName2);
  TTree* calib = (TTree*)calibf1->Get("calibevt");
  vector<float>* charge;
  vector<float>* htime;
  vector<int>* calib_PMTID;
  float totalcharge;
  int trigt1;
  int trigt2;
  charge = NULL;
  htime = NULL;
  calib_PMTID = NULL;

  calib->SetBranchAddress("Charge", &charge);
  calib->SetBranchAddress("Time", &htime);
  calib->SetBranchAddress("PMTID", &calib_PMTID);
  calib->SetBranchAddress("TotalPE", &totalcharge);
  calib->SetBranchAddress("TrigTimeSec", &trigt1);
  calib->SetBranchAddress("TrigTimeNanoSec", &trigt2);

  TString fileName3 = datapath + "/ES/elec_user_" + ID + ".root";  //--J21
  TFile* elecf1 = TFile::Open(fileName3);
  TTree* elec = (TTree*)elecf1->Get("eventindex");
  int eventid;
  int nevents;
  vector<int>* entries;
  entries = NULL;

  elec->SetBranchAddress("eventid", &eventid);
  elec->SetBranchAddress("nevents", &nevents);
  elec->SetBranchAddress("entries", &entries);

  int ne3 = elec->GetEntries();

  TString recFileName = datapath + "/REC/rec_user_" + ID + ".root";  //--J21

  TFile* recf1 = TFile::Open(recFileName);
  TTree* rec = (TTree*)recf1->Get("TRec");
  rec->SetBranchAddress("recx", &recx);
  rec->SetBranchAddress("recy", &recy);
  rec->SetBranchAddress("recz", &recz);

  PMTPosition();
  ReadPMTdata();

  //TString outName1 = datapath + "/Ana/dsnbhist_" + ID + ".root";  //--J21
  //TString outName2 = datapath + "/Ana/dsnbfit_" + ID + ".root";   //--J21

  TString outName1 = "./dsnbhist_" + ID + ".root";  //--J21
  TString outName2 = "./dsnbfit_" + ID + ".root";   //--J21

  cout << "===> out file : " << outName1 << endl;
  cout << "===> out file : " << outName2 << endl;
  TFile* f_psd1 = new TFile(outName1, "recreate");
  TFile* f_psd2 = new TFile(outName2, "recreate");
  TTree* t_psd = new TTree("psdtree", "psd tree");
  int file_id;
  int evtID_o;
  int eleID_o;
  double Edep_o;
  double Eqen_o;
  double Xdep_o;
  double Ydep_o;
  double Zdep_o;
  double Xrec_o;
  double Yrec_o;
  double Zrec_o;
  double nPE_o;
  double nPEcalib_o;
  const int nsteps = 30;
  int nsteps_o;
  double PSD_fs01[nsteps];
  double PSD_fs02[nsteps];

  double tau1_fs1;
  double tau2_fs1;
  // double tau3_fs1;
  double eta1_fs1;
  // double eta2_fs1;
  double NN_fs1;
  double Ndark_fs1;
  double chi2_fs1;
  double ndf_fs1;

  double tau1_fs2;
  double tau2_fs2;
  // double tau3_fs2;
  double eta1_fs2;
  // double eta2_fs2;
  double NN_fs2;
  double Ndark_fs2;
  double chi2_fs2;
  double ndf_fs2;

  double tau1_mcp_fs1;
  double tau2_mcp_fs1;
  // double tau3_mcp_fs1;
  double eta1_mcp_fs1;
  // double eta2_mcp_fs1;
  double NN_mcp_fs1;
  double Ndark_mcp_fs1;
  double chi2_mcp_fs1;
  double ndf_mcp_fs1;

  double tau1_mcp_fs2;
  double tau2_mcp_fs2;
  // double tau3_mcp_fs2;
  double eta1_mcp_fs2;
  // double eta2_mcp_fs2;
  double NN_mcp_fs2;
  double Ndark_mcp_fs2;
  double chi2_mcp_fs2;
  double ndf_mcp_fs2;

  double tau1_ham_fs1;
  double tau2_ham_fs1;
  // double tau3_ham_fs1;
  double eta1_ham_fs1;
  // double eta2_ham_fs1;
  double NN_ham_fs1;
  double Ndark_ham_fs1;
  double chi2_ham_fs1;
  double ndf_ham_fs1;

  double tau1_ham_fs2;
  double tau2_ham_fs2;
  // double tau3_ham_fs2;
  double eta1_ham_fs2;
  // double eta2_ham_fs2;
  double NN_ham_fs2;
  double Ndark_ham_fs2;
  double chi2_ham_fs2;
  double ndf_ham_fs2;

  int nsels_o;
  double w1_fs1[3];
  double w2_fs1[3];
  double w1_fs2[3];
  double w2_fs2[3];

  double w1new_fs1;  //----50% of peak
  double w2new_fs1;  //----50% of peak
  double w1new_fs2;  //----50% of peak
  double w2new_fs2;  //----50% of peak

  t_psd->Branch("file_id", &file_id, "file_id/I");
  t_psd->Branch("evtID_o", &evtID_o, "evtID_o/I");
  t_psd->Branch("eleID_o", &eleID_o, "eleID_o/I");
  t_psd->Branch("Edep_o", &Edep_o, "Edep_o/D");
  t_psd->Branch("Eqen_o", &Eqen_o, "Eqen_o/D");
  t_psd->Branch("Xdep_o", &Xdep_o, "Xdep_o/D");
  t_psd->Branch("Ydep_o", &Ydep_o, "Ydep_o/D");
  t_psd->Branch("Zdep_o", &Zdep_o, "Zdep_o/D");
  t_psd->Branch("Xrec_o", &Xrec_o, "Xrec_o/D");
  t_psd->Branch("Yrec_o", &Yrec_o, "Yrec_o/D");
  t_psd->Branch("Zrec_o", &Zrec_o, "Zrec_o/D");
  t_psd->Branch("nPE_o", &nPE_o, "nPE_o/D");
  t_psd->Branch("nPEcalib_o", &nPEcalib_o, "nPEcalib_o/D");
  t_psd->Branch("nsteps_o", &nsteps_o, "nsteps_o/I");
  t_psd->Branch("PSD_fs01", PSD_fs01, "PSD_fs01[nsteps_o]/D");
  t_psd->Branch("PSD_fs02", PSD_fs02, "PSD_fs02[nsteps_o]/D");

  t_psd->Branch("tau1_fs1", &tau1_fs1, "tau1_fs1/D");
  t_psd->Branch("tau2_fs1", &tau2_fs1, "tau2_fs1/D");
  // t_psd->Branch("tau3_fs1", &tau3_fs1, "tau3_fs1/D");
  t_psd->Branch("eta1_fs1", &eta1_fs1, "eta1_fs1/D");
  // t_psd->Branch("eta2_fs1", &eta2_fs1, "eta2_fs1/D");
  t_psd->Branch("NN_fs1", &NN_fs1, "NN_fs1/D");
  t_psd->Branch("Ndark_fs1", &Ndark_fs1, "Ndark_fs1/D");
  t_psd->Branch("chi2_fs1", &chi2_fs1, "chi2_fs1/D");
  t_psd->Branch("ndf_fs1", &ndf_fs1, "ndf_fs1/D");

  t_psd->Branch("tau1_fs2", &tau1_fs2, "tau1_fs2/D");
  t_psd->Branch("tau2_fs2", &tau2_fs2, "tau2_fs2/D");
  // t_psd->Branch("tau3_fs2", &tau3_fs2, "tau3_fs2/D");
  t_psd->Branch("eta1_fs2", &eta1_fs2, "eta1_fs2/D");
  // t_psd->Branch("eta2_fs2", &eta2_fs2, "eta2_fs2/D");
  t_psd->Branch("NN_fs2", &NN_fs2, "NN_fs2/D");
  t_psd->Branch("Ndark_fs2", &Ndark_fs2, "Ndark_fs2/D");
  t_psd->Branch("chi2_fs2", &chi2_fs2, "chi2_fs2/D");
  t_psd->Branch("ndf_fs2", &ndf_fs2, "ndf_fs2/D");

  t_psd->Branch("tau1_mcp_fs1", &tau1_mcp_fs1, "tau1_mcp_fs1/D");
  t_psd->Branch("tau2_mcp_fs1", &tau2_mcp_fs1, "tau2_mcp_fs1/D");
  // t_psd->Branch("tau3_mcp_fs1", &tau3_mcp_fs1, "tau3_mcp_fs1/D");
  t_psd->Branch("eta1_mcp_fs1", &eta1_mcp_fs1, "eta1_mcp_fs1/D");
  // t_psd->Branch("eta2_mcp_fs1", &eta2_mcp_fs1, "eta2_mcp_fs1/D");
  t_psd->Branch("NN_mcp_fs1", &NN_mcp_fs1, "NN_mcp_fs1/D");
  t_psd->Branch("Ndark_mcp_fs1", &Ndark_mcp_fs1, "Ndark_mcp_fs1/D");
  t_psd->Branch("chi2_mcp_fs1", &chi2_mcp_fs1, "chi2_mcp_fs1/D");
  t_psd->Branch("ndf_mcp_fs1", &ndf_mcp_fs1, "ndf_mcp_fs1/D");

  t_psd->Branch("tau1_mcp_fs2", &tau1_mcp_fs2, "tau1_mcp_fs2/D");
  t_psd->Branch("tau2_mcp_fs2", &tau2_mcp_fs2, "tau2_mcp_fs2/D");
  // t_psd->Branch("tau3_mcp_fs2", &tau3_mcp_fs2, "tau3_mcp_fs2/D");
  t_psd->Branch("eta1_mcp_fs2", &eta1_mcp_fs2, "eta1_mcp_fs2/D");
  // t_psd->Branch("eta2_mcp_fs2", &eta2_mcp_fs2, "eta2_mcp_fs2/D");
  t_psd->Branch("NN_mcp_fs2", &NN_mcp_fs2, "NN_mcp_fs2/D");
  t_psd->Branch("Ndark_mcp_fs2", &Ndark_mcp_fs2, "Ndark_mcp_fs2/D");
  t_psd->Branch("chi2_mcp_fs2", &chi2_mcp_fs2, "chi2_mcp_fs2/D");
  t_psd->Branch("ndf_mcp_fs2", &ndf_mcp_fs2, "ndf_mcp_fs2/D");

  t_psd->Branch("tau1_ham_fs1", &tau1_ham_fs1, "tau1_ham_fs1/D");
  t_psd->Branch("tau2_ham_fs1", &tau2_ham_fs1, "tau2_ham_fs1/D");
  // t_psd->Branch("tau3_ham_fs1", &tau3_ham_fs1, "tau3_ham_fs1/D");
  t_psd->Branch("eta1_ham_fs1", &eta1_ham_fs1, "eta1_ham_fs1/D");
  // t_psd->Branch("eta2_ham_fs1", &eta2_ham_fs1, "eta2_ham_fs1/D");
  t_psd->Branch("NN_ham_fs1", &NN_ham_fs1, "NN_ham_fs1/D");
  t_psd->Branch("Ndark_ham_fs1", &Ndark_ham_fs1, "Ndark_ham_fs1/D");
  t_psd->Branch("chi2_ham_fs1", &chi2_ham_fs1, "chi2_ham_fs1/D");
  t_psd->Branch("ndf_ham_fs1", &ndf_ham_fs1, "ndf_ham_fs1/D");

  t_psd->Branch("tau1_ham_fs2", &tau1_ham_fs2, "tau1_ham_fs2/D");
  t_psd->Branch("tau2_ham_fs2", &tau2_ham_fs2, "tau2_ham_fs2/D");
  // t_psd->Branch("tau3_ham_fs2", &tau3_ham_fs2, "tau3_ham_fs2/D");
  t_psd->Branch("eta1_ham_fs2", &eta1_ham_fs2, "eta1_ham_fs2/D");
  // t_psd->Branch("eta2_ham_fs2", &eta2_ham_fs2, "eta2_ham_fs2/D");
  t_psd->Branch("NN_ham_fs2", &NN_ham_fs2, "NN_ham_fs2/D");
  t_psd->Branch("Ndark_ham_fs2", &Ndark_ham_fs2, "Ndark_ham_fs2/D");
  t_psd->Branch("chi2_ham_fs2", &chi2_ham_fs2, "chi2_ham_fs2/D");
  t_psd->Branch("ndf_ham_fs2", &ndf_ham_fs2, "ndf_ham_fs2/D");

  t_psd->Branch("nsels_o", &nsels_o, "nsels_o/I");
  t_psd->Branch("w1_fs1", w1_fs1, "w1_fs1[nsels_o]/D");
  t_psd->Branch("w2_fs1", w2_fs1, "w2_fs1[nsels_o]/D");
  t_psd->Branch("w1_fs2", w1_fs2, "w1_fs2[nsels_o]/D");
  t_psd->Branch("w2_fs2", w2_fs2, "w2_fs2[nsels_o]/D");

  t_psd->Branch("w1new_fs1", &w1new_fs1, "w1new_fs1/D");
  t_psd->Branch("w2new_fs1", &w2new_fs1, "w2new_fs1/D");
  t_psd->Branch("w1new_fs2", &w1new_fs2, "w1new_fs2/D");
  t_psd->Branch("w2new_fs2", &w2new_fs2, "w2new_fs2/D");

  cout << "-------> EJie " << endl;
  vector<struct Event> m_timeBuffer;
  m_timeBuffer.clear();

  vector<struct Event> m_ibdlike_p;
  m_ibdlike_p.clear();

  vector<struct eleEvent> m_elecID;
  vector<struct eleEvent> m_elec_temID;
  vector<struct eleEvent> m_elecIBDlikeID;

  TH1F* pmtime_fs1_lowR_lowE = new TH1F("pmtime_fs1_lowR_lowE", "pmtime_fs1_lowR_lowE", 2000, -200, 1800);
  TH1F* pmtime_fs1_lowR_highE = new TH1F("pmtime_fs1_lowR_highE", "pmtime_fs1_lowR_highE", 2000, -200, 1800);
  TH1F* pmtime_fs1_highR_lowE = new TH1F("pmtime_fs1_highR_lowE", "pmtime_fs1_highR_lowE", 2000, -200, 1800);
  TH1F* pmtime_fs1_highR_highE = new TH1F("pmtime_fs1_highR_highE", "pmtime_fs1_highR_highE", 2000, -200, 1800);

  TH1F* pmtime_fs2_lowR_lowE = new TH1F("pmtime_fs2_lowR_lowE", "pmtime_fs2_lowR_lowE", 2000, -200, 1800);
  TH1F* pmtime_fs2_lowR_highE = new TH1F("pmtime_fs2_lowR_highE", "pmtime_fs2_lowR_highE", 2000, -200, 1800);
  TH1F* pmtime_fs2_highR_lowE = new TH1F("pmtime_fs2_highR_lowE", "pmtime_fs2_highR_lowE", 2000, -200, 1800);
  TH1F* pmtime_fs2_highR_highE = new TH1F("pmtime_fs2_highR_highE", "pmtime_fs2_highR_highE", 2000, -200, 1800);

  TH1F* pmtime_fsham1_lowR_lowE = new TH1F("pmtime_fsham1_lowR_lowE", "pmtime_fsham1_lowR_lowE", 4000, -20, 20);
  TH1F* pmtime_fsham1_lowR_highE = new TH1F("pmtime_fsham1_lowR_highE", "pmtime_fsham1_lowR_highE", 4000, -20, 20);
  TH1F* pmtime_fsham1_highR_lowE = new TH1F("pmtime_fsham1_highR_lowE", "pmtime_fsham1_highR_lowE", 4000, -20, 20);
  TH1F* pmtime_fsham1_highR_highE = new TH1F("pmtime_fsham1_highR_highE", "pmtime_fsham1_highR_highE", 4000, -20, 20);

  TH1F* pmtime_fsham2_lowR_lowE = new TH1F("pmtime_fsham2_lowR_lowE", "pmtime_fsham2_lowR_lowE", 4000, -20, 20);
  TH1F* pmtime_fsham2_lowR_highE = new TH1F("pmtime_fsham2_lowR_highE", "pmtime_fsham2_lowR_highE", 4000, -20, 20);
  TH1F* pmtime_fsham2_highR_lowE = new TH1F("pmtime_fsham2_highR_lowE", "pmtime_fsham2_highR_lowE", 4000, -20, 20);
  TH1F* pmtime_fsham2_highR_highE = new TH1F("pmtime_fsham2_highR_highE", "pmtime_fsham2_highR_highE", 4000, -20, 20);

  double c = 2.99792458e2;  // mm/ns
  double vlab = c / 1.578;

  int tailtimecut[nsteps] = {0};
  int starttime = 0.;  // ns

  for (int i = 0; i < nsteps; i++) {
    tailtimecut[i] = starttime + 10 + i * 10;
  }

  //nentries = elec->GetEntries();
  
  // Debug....
  nentries = 20;

  for (int ii = 0; ii < nentries; ii++) {
    // for (int ii = 0; ii < 50; ii++) {
    elec->GetEntry(ii);
    calib->GetEntry(ii);
    double t1 = trigt1;
    double t2 = trigt2;

    double ttime = t1 * 1.e3 + t2 * 1.e-6;  // ms
    cout << "-----> test trigtime = " << ttime << endl;

    for (int jj = 0; jj < nevents; jj++) {
      struct eleEvent eleEvt;
      eleEvt.sim_evt = (*entries)[jj];
      eleEvt.ele_evt = eventid;
      eleEvt.status_evt = 0;
      eleEvt.trigTime_evt = ttime;
      // if (ttime < 0) continue;
      // if(totalcharge<1000.) continue;
      if (ttime < -1.e10) continue;
      m_elecID.push_back(eleEvt);
    }
  }

  cout << "---------- m_elecID size =  " << m_elecID.size() << endl;

  for (int i = 0; i < m_elecID.size(); i++) {
    struct eleEvent tem1Evt = m_elecID[i];
    cout << "tem1 test : " << tem1Evt.sim_evt << "\t" << tem1Evt.ele_evt << "\t" << tem1Evt.status_evt << "\t" << tem1Evt.trigTime_evt << endl;
    if (tem1Evt.status_evt == 1) continue;
    int currSimID = tem1Evt.sim_evt;
    m_elec_temID.clear();
    for (int j = 0; j < m_elecID.size(); j++) {
      struct eleEvent tem2Evt = m_elecID[j];
      if (tem2Evt.status_evt == 1) continue;
      if (tem2Evt.sim_evt == currSimID) {
        m_elecID[j].status_evt = 1;
        m_elec_temID.push_back(tem2Evt);
      }
    }
    if (m_elec_temID.size() == 1) {
      m_elecIBDlikeID.push_back(m_elec_temID[0]);
    }
    if (m_elec_temID.size() > 1) {
      double dt = m_elec_temID[1].trigTime_evt - m_elec_temID[0].trigTime_evt;
      if (dt > 1.) {
        m_elecIBDlikeID.push_back(m_elec_temID[0]);
      }
    }
    m_elec_temID.clear();
  }
  m_elecID.clear();

  cout << "**************** m_elecIBDlikeID size = " << m_elecIBDlikeID.size() << endl;

  for (int ii = 0; ii < m_elecIBDlikeID.size(); ii++) {
    struct eleEvent temEvt = m_elecIBDlikeID[ii];
    int simevt_id = temEvt.sim_evt;
    int eleevt_id = temEvt.ele_evt;
    depT->GetEntry(simevt_id);
    cout << "-------> EventID : " << evtID_n << endl;
    evtID_o = 0, Edep_o = 0., Eqen_o = 0, Xdep_o = 0, Ydep_o = 0, Zdep_o = 0;
    Xrec_o = 0, Yrec_o = 0, Zrec_o = 0., nPE_o = 0;
    nPEcalib_o = 0;
    for (int in = 0; in < 30; in++) {
      PSD_fs01[in] = 0.;
      PSD_fs02[in] = 0.;
    }

    int t_PDG = 0;
    double t_x = 0., t_y = 0., t_z = 0.;
    double t_t = 0.;
    double t_edep = 0.;
    double t_qe = 0.;
    double t_tcap = 0.;
    double t_win = 300.;
    double dr3 = 0.;

    for (int stepN = 0; stepN < stepNumber; stepN++) {
      if (stepN == 0) {
        t_t = GlobalTime[stepN];
        t_x = Positionx[stepN] * EnergyDeposit[stepN];
        t_y = Positiony[stepN] * EnergyDeposit[stepN];
        t_z = Positionz[stepN] * EnergyDeposit[stepN];
        t_edep = EnergyDeposit[stepN];
        t_qe = QEnergyDeposit[stepN];
        t_PDG = 0;
        t_tcap = 0;
        if (Captime[stepN] > 0 && PDG[stepN] == 2112) {
          t_PDG = 2112;
          t_tcap = Captime[stepN];
        }
      } else {
        double dt = GlobalTime[stepN] - t_t;
        if (dt < 0) cout << " Warning  ::: Call Jie : please check input root file" << endl;
        if (dt > 0 && dt < t_win) {
          t_x += Positionx[stepN] * EnergyDeposit[stepN];
          t_y += Positiony[stepN] * EnergyDeposit[stepN];
          t_z += Positionz[stepN] * EnergyDeposit[stepN];
          t_edep += EnergyDeposit[stepN];
          t_qe += QEnergyDeposit[stepN];
          if (Captime[stepN] > 0 && PDG[stepN] == 2112) {
            if (t_PDG == 2112) cout << "Warning  ::: Call Jie : please check input root file for double neutrons" << endl;
            t_PDG = 2112;
            t_tcap = Captime[stepN];
          }
        } else if (dt >= t_win) {

          struct Event newEvt;
          newEvt.id_evt = -1;
          newEvt.id_evt = evtID_n;
          newEvt.pdg_evt = 0;
          newEvt.pdg_evt = t_PDG;
          newEvt.x_evt = 0.;
          newEvt.x_evt = t_x / t_edep;
          newEvt.y_evt = 0.;
          newEvt.y_evt = t_y / t_edep;
          newEvt.z_evt = 0.;
          newEvt.z_evt = t_z / t_edep;
          newEvt.t_evt = 0.;
          newEvt.t_evt = t_t;
          newEvt.edep_evt = 0.;
          newEvt.edep_evt = t_edep;
          newEvt.qe_evt = 0.;
          newEvt.qe_evt = t_qe;
          m_timeBuffer.push_back(newEvt);

          t_PDG = 0;
          t_tcap = 0;
          t_t = GlobalTime[stepN];
          t_x = Positionx[stepN] * EnergyDeposit[stepN];
          t_y = Positiony[stepN] * EnergyDeposit[stepN];
          t_z = Positionz[stepN] * EnergyDeposit[stepN];
          t_edep = EnergyDeposit[stepN];
          t_qe = QEnergyDeposit[stepN];
          if (Captime[stepN] > 0 && PDG[stepN] == 2112) {
            t_PDG = 2112;
            t_tcap = Captime[stepN];
          }
        }
      }
      if (stepN == stepNumber - 1) {
        struct Event newEvt;
        newEvt.id_evt = -1;
        newEvt.id_evt = evtID_n;
        newEvt.pdg_evt = 0;
        newEvt.pdg_evt = t_PDG;
        newEvt.x_evt = 0.;
        newEvt.x_evt = t_x / t_edep;
        newEvt.y_evt = 0.;
        newEvt.y_evt = t_y / t_edep;
        newEvt.z_evt = 0.;
        newEvt.z_evt = t_z / t_edep;
        newEvt.t_evt = 0.;
        newEvt.t_evt = t_t;
        newEvt.edep_evt = 0.;
        newEvt.edep_evt = t_edep;
        newEvt.qe_evt = 0.;
        newEvt.qe_evt = t_qe;
        m_timeBuffer.push_back(newEvt);
      }
    }
    if (m_timeBuffer.size() >= 1) {
      //-----------------> test
      cout << "-----------------> evtid ====  " << evtID_n << endl;
      for (int jj = 0; jj < m_timeBuffer.size(); jj++) {
        struct Event pEvt = m_timeBuffer[jj];
        cout << pEvt.pdg_evt << "\t" << pEvt.qe_evt << "\t" << pEvt.t_evt << endl;
      }
      cout << "-------------------- test end ---------" << endl;
      struct Event pEvt = m_timeBuffer[0];
      evt->GetEntry(evtID_n);
      geninfo->GetEntry(evtID_n);
      rec->GetEntry(eleevt_id);
      Xrec_o = recx;
      Yrec_o = recy;
      Zrec_o = recz;
      dr3 = pow(sqrt(Xrec_o * Xrec_o * 1.e-6 + Yrec_o * Yrec_o * 1.e-6 + Zrec_o * Zrec_o * 1.e-6), 3);
      cout << "-----> test 2" << dr3 << endl;

      nPE_o = 0.;
      double nhits = 0.;
      file_id = fileNo;
      evtID_o = evtID_n;
      Edep_o = pEvt.edep_evt;
      Eqen_o = pEvt.qe_evt;
      Xdep_o = pEvt.x_evt;
      Ydep_o = pEvt.y_evt;
      Zdep_o = pEvt.z_evt;
      nsteps_o = nsteps;
      double dr_recvsdep = pow(Xrec_o - Xdep_o, 2) + pow(Yrec_o - Ydep_o, 2) + pow(Zrec_o - Zdep_o, 2);
      dr_recvsdep = sqrt(dr_recvsdep) * 1e-3;
      if (pEvt.qe_evt > 10. && dr_recvsdep < 1.) {

        double dr3cut = pow(17.7, 3);  //--->dr = 17.7m
        int eleID = eleevt_id;
        eleID_o = eleID;
        cout << "------> test ele_evt = " << eleID << "\t"
             << "\t  evtID_n = " << evtID_n << endl;
        TString hhnn1 = "timetest_fs1_";
        hhnn1 += evtID_n;
        TH1F* timetest1_fs1 = new TH1F(hhnn1, hhnn1, 2000, -200, 1800);
        TH1F* timetest1new_fs1 = new TH1F("timetest1new_fs1", "timetest1new_fs1", 2000, -200, 1800);
        TH1F* timetest3_fs1 = new TH1F("timetest3_fs1", "timetest3_fs1", 2000, -200, 1800);
        TH1F* timetest4_fs1 = new TH1F("timetest4_fs1", "timetest4_fs1", 800, 0, 800);
        TH1F* timetest5_fs1 = new TH1F("timetest5_fs1", "timetest5_fs1", 200, -200, 0);

        //-----------------MCP PMT
        TH1F* timetest1_fsmcp1 = new TH1F("timetest1_fsmcp1", "timetest1_fsmcp1", 2000, -200, 1800);
        TH1F* timetest4_fsmcp1 = new TH1F("timetest4_fsmcp1", "timetest4_fsmcp1", 800, 0, 800);
        TH1F* timetest5_fsmcp1 = new TH1F("timetest5_fsmcp1", "timetest5_fsmcp1", 200, -200, 0);
        TH1F* timetest1_fsmcp2 = new TH1F("timetest1_fsmcp2", "timetest1_fsmcp2", 2000, -200, 1800);
        TH1F* timetest4_fsmcp2 = new TH1F("timetest4_fsmcp2", "timetest4_fsmcp2", 800, 0, 800);
        TH1F* timetest5_fsmcp2 = new TH1F("timetest5_fsmcp2", "timetest5_fsmcp2", 200, -200, 0);

        //----------------only Hamamtsu PMTs
        TH1F* timetest1_fsham1 = new TH1F("timetest1_fsham1", "timetest1_fsham1", 2000, -200, 1800);
        TH1F* timetest1new_fsham1 = new TH1F("timetest1new_fsham1", "timetest1new_fsham1", 2000, -200, 1800);
        TH1F* timetest2_fsham1 = new TH1F("timetest2_fsham1", "timetest2_fsham1", 4000, -20, 20);
        TH1F* timetest2new_fsham1 = new TH1F("timetest2new_fsham1", "timetest2new_fsham1", 40, -20, 20);
        TH1F* timetest3_fsham1 = new TH1F("timetest3_fsham1", "timetest3_fsham1", 2000, -200, 1800);
        TH1F* timetest4_fsham1 = new TH1F("timetest4_fsham1", "timetest4_fsham1", 800, 0, 800);
        TH1F* timetest5_fsham1 = new TH1F("timetest5_fsham1", "timetest5_fsham1", 200, -200, 0);
        vector<double> itime2;
        itime2.clear();
        TString hhnn2 = "timetest_fs2_";  //---------- w/ charge weight
        hhnn2 += evtID_n;
        TH1F* timetest1_fs2 = new TH1F(hhnn2, hhnn2, 2000, -200, 1800);
        TH1F* timetest1new_fs2 = new TH1F("timetest1new_fs2", "timetest1new_fs2", 2000, -200, 1800);
        TH1F* timetest3_fs2 = new TH1F("timetest3_fs2", "timetest3_fs2", 2000, -200, 1800);
        TH1F* timetest4_fs2 = new TH1F("timetest4_fs2", "timetest4_fs2", 800, 0, 800);
        TH1F* timetest5_fs2 = new TH1F("timetest5_fs2", "timetest5_fs2", 200, -200, 0);

        //----------------only Hamamtsu PMTs
        TH1F* timetest1_fsham2 = new TH1F("timetest1_fsham2", "timetest1_fsham2", 2000, -200, 1800);
        TH1F* timetest1new_fsham2 = new TH1F("timetest1new_fsham2", "timetest1new_fsham2", 2000, -200, 1800);
        TH1F* timetest2_fsham2 = new TH1F("timetest2_fsham2", "timetest2_fsham2", 4000, -20, 20);
        TH1F* timetest2new_fsham2 = new TH1F("timetest2new_fsham2", "timetest2new_fsham2", 40, -20, 20);
        TH1F* timetest3_fsham2 = new TH1F("timetest3_fsham2", "timetest3_fsham2", 2000, -200, 1800);
        TH1F* timetest4_fsham2 = new TH1F("timetest4_fsham2", "timetest4_fsham2", 800, 0, 800);
        TH1F* timetest5_fsham2 = new TH1F("timetest5_fsham2", "timetest5_fsham2", 200, -200, 0);
        TTree* treeAll_tem = new TTree("treeAll_tem", "treeAll_tem");
        TH1F* timeCheck01 = new TH1F("timeCheck01", "timeCheck01", 2200, -200, 2000);
        double tt_0;
        double ncharge;
        treeAll_tem->Branch("tt_0", &tt_0, "tt_0/D");
        treeAll_tem->Branch("ncharge", &ncharge, "ncharge/D");

        calib->GetEntry(eleID);
        nPEcalib_o = 0;
        double nPEcalibham_o = 0;
        for (int j = 0; j < calib_PMTID->size(); j++) {
          int pmtid = (((*calib_PMTID)[j]) & 0x00FFFF00) >> 8;
          if (pmtid > nPMTs) continue;
          double pmtpos_x = PMTpos_x[pmtid];
          double pmtpos_y = PMTpos_y[pmtid];
          double pmtpos_z = PMTpos_z[pmtid];

          double npe = (*charge)[j];
          // if (npe < 1.5) npe = 1;
          double dr1 = sqrt(pow((pmtpos_x - Xrec_o), 2) + pow((pmtpos_y - Yrec_o), 2) + pow((pmtpos_z - Zrec_o), 2));
          double ttof1 = dr1 / vlab;
          double tsignal = (*htime)[j];
          double ttt = timeOffset[pmtid];
          double thit = tsignal;
          double tpep1 = thit - ttof1;
          timetest1_fs1->Fill(tpep1);
          timetest1new_fs1->Fill(tpep1);
          timetest1_fs2->Fill(tpep1, npe);
          timetest1new_fs2->Fill(tpep1, npe);
          nPEcalib_o = nPEcalib_o + npe;
          if (isHamamatsu[pmtid] == 0) {
            timetest1_fsmcp1->Fill(tpep1);
            timetest1_fsmcp2->Fill(tpep1, npe);
          } else {
            timetest1_fsham1->Fill(tpep1);
            timetest1new_fsham1->Fill(tpep1);
            nPEcalibham_o = nPEcalibham_o + npe;
            itime2.push_back(tpep1);
            timetest1_fsham2->Fill(tpep1, npe);
            timetest1new_fsham2->Fill(tpep1, npe);
          }
        }
        double timetag2 = 0., timetag3 = 0., timetag4 = 0., timetag5 = 0., timetag6 = 0., timetag7 = 0.;
        timetag2 = GetTimetag(timetest1_fs1);
        timetag3 = GetTimetag(timetest1_fs2);
        timetag4 = GetTimetag(timetest1_fsham1);
        timetag5 = GetTimetag(timetest1_fsham2);
        timetag6 = GetTimetag(timetest1_fsmcp1);
        timetag7 = GetTimetag(timetest1_fsmcp2);
        cout << "------------------> test timetag2 = " << timetag2 << endl;
        cout << "----------------> timetag3 = " << timetag3 << endl;
        cout << "------------------> test timetag4 = " << timetag4 << endl;
        cout << "----------------> timetag5 = " << timetag5 << endl;
        cout << "----------------> timetag6 = " << timetag6 << endl;
        cout << "----------------> timetag7 = " << timetag7 << endl;
        double hpeak_hamfs1 = 0.;
        double hpeak_hamfs2 = 0.;
        cout << "============================================= Evt = " << evtID_n << endl;
        timetest4_fs2->Sumw2();
        for (int j = 0; j < calib_PMTID->size(); j++) {
          int pmtid = (((*calib_PMTID)[j]) & 0x00FFFF00) >> 8;
          if (pmtid > nPMTs) continue;
          double pmtpos_x = PMTpos_x[pmtid];
          double pmtpos_y = PMTpos_y[pmtid];
          double pmtpos_z = PMTpos_z[pmtid];

          double npe = (*charge)[j];
          // if (npe < 1.5) npe = 1;
          double dr1 = sqrt(pow((pmtpos_x - Xrec_o), 2) + pow((pmtpos_y - Yrec_o), 2) + pow((pmtpos_z - Zrec_o), 2));
          double ttof1 = dr1 / vlab;
          double tsignal = (*htime)[j];
          double ttt = timeOffset[pmtid];
          // double thit = tsignal - ttt;
          double thit = tsignal;
          double tpep1 = thit - ttof1;
          double tt = 0.;
          tt = tpep1 - timetag3 + 1;
          timeCheck01->Fill(tt);
          if (tt >= 0. && tt < 1000) {
            timetest4_fs2->Fill(tt, npe);
            tt_0 = 0.;
            tt_0 = tt;
            ncharge = 0.;
            ncharge = npe;
            treeAll_tem->Fill();
          }
          if (tt >= -200. && tt < 0) {
            timetest5_fs2->Fill(tt, npe);
          }
          tt = tpep1 - timetag2 + 1;
          if (tt >= 0. && tt < 800) {
            timetest4_fs1->Fill(tt);
          }
          if (tt >= -200. && tt < 0) {
            timetest5_fs1->Fill(tt);
          }
          if (isHamamatsu[pmtid] == 0) {
            tt = tpep1 - timetag6 + 1;
            if (tt >= 0. && tt < 800) {
              timetest4_fsmcp1->Fill(tt);
            }
            if (tt >= -200. && tt < 0) {
              timetest5_fsmcp1->Fill(tt);
            }
            tt = tpep1 - timetag7 + 1;
            if (tt >= 0. && tt < 800) {
              timetest4_fsmcp2->Fill(tt, npe);
            }
            if (tt >= -200. && tt < 0) {
              timetest5_fsmcp2->Fill(tt);
            }
          } else {

            tt = tpep1 - timetag4 + 1;
            if (tt >= 0. && tt < 800.) {
              timetest4_fsham1->Fill(tt);
            }
            if (tt >= -200. && tt < 0) {
              timetest5_fsham1->Fill(tt);
            }
            tt = tpep1 - timetag5 + 1;
            if (tt > -20. && tt < 20.) {
              timetest2_fsham2->Fill(tt, npe);
              timetest2new_fsham2->Fill(tt, npe);
            }
            if (tt >= 0. && tt < 800.) {
              timetest4_fsham2->Fill(tt, npe);
            }
            if (tt >= -200. && tt < 0) {
              timetest5_fsham2->Fill(tt);
            }
          }
        }

        for (int it = 0; it < itime2.size(); it++) {
          double tpep2 = itime2[it] - timetag4 + 1;
          if (tpep2 > -20. && tpep2 < 20.) {
            timetest2_fsham1->Fill(tpep2);
            timetest2new_fsham1->Fill(tpep2);
          }
        }
        itime2.clear();
        timetest2_fsham1->Scale(1. / timetest2_fsham1->Integral());
        timetest2_fsham2->Scale(1. / timetest2_fsham2->Integral());

        //-------------calculate tail to total
        for (int tt = 0; tt < nsteps; tt++) {
          PSD_fs01[tt] = GetTail2total(timetest1new_fs1, timetag2, tt);
          PSD_fs02[tt] = GetTail2total(timetest1new_fs2, timetag3, tt);
          // cout << "------test \t tt = " << tt << "\t rpsd = " << PSD_fs01[tt] << endl;
        }

        //------------calculate w1new and w2new
        hpeak_hamfs1 = GetPeakvalue(timetest1_fsham1);
        hpeak_hamfs2 = GetPeakvalue(timetest1_fsham2);

        double wcut1 = hpeak_hamfs1 * 0.5;
        double wcut2 = hpeak_hamfs2 * 0.5;
        w1new_fs1 = 0.;
        w2new_fs1 = 0.;
        w1new_fs1 = GetW1new(timetest2new_fsham1, wcut1);
        w2new_fs1 = GetW2new(timetest2new_fsham1, wcut1);
        w1new_fs2 = 0.;
        w2new_fs2 = 0.;
        w1new_fs2 = GetW1new(timetest2new_fsham2, wcut1);
        w2new_fs2 = GetW2new(timetest2new_fsham2, wcut1);

        // cout << "-----> test new w || w1new = " << w1new_fs1 << "\t || w2new = " << w2new_fs1 << endl;

        //------------calculate w1 and w2
        nsels_o = 3;
        for (int ss = 0; ss < nsels_o; ss++) {
          w1_fs1[ss] = 0.;
          w2_fs1[ss] = 0.;
          w1_fs2[ss] = 0.;
          w2_fs2[ss] = 0.;
          w1_fs1[ss] = GetW1(timetest2_fsham1, ss);
          w2_fs1[ss] = GetW2(timetest2_fsham1, ss);
          w1_fs2[ss] = GetW1(timetest2_fsham2, ss);
          w2_fs2[ss] = GetW2(timetest2_fsham2, ss);
        }
        // cout << "-----> test new w || w1 = " << w1_fs1[0] << "\t || w2 = " << w2_fs1[0] << endl;

        //--------------------Fitting

        tau1_fs1 = 0., tau1_fs2 = 0., tau1_mcp_fs1 = 0., tau1_mcp_fs2 = 0., tau1_ham_fs1 = 0., tau1_ham_fs2 = 0.;
        tau2_fs1 = 0., tau2_fs2 = 0., tau2_mcp_fs1 = 0., tau2_mcp_fs2 = 0., tau2_ham_fs1 = 0., tau2_ham_fs2 = 0.;
        // tau3_fs1 = 0., tau3_fs2 = 0., tau3_mcp_fs1 = 0., tau3_mcp_fs2 = 0., tau3_ham_fs1 = 0., tau3_ham_fs2 = 0.;
        eta1_fs1 = 0., eta1_fs2 = 0., eta1_mcp_fs1 = 0., eta1_mcp_fs2 = 0., eta1_ham_fs1 = 0., eta1_ham_fs2 = 0.;
        // eta2_fs1 = 0., eta2_fs2 = 0., eta2_mcp_fs1 = 0., eta2_mcp_fs2 = 0., eta2_ham_fs1 = 0., eta2_ham_fs2 = 0.;
        NN_fs1 = 0., NN_fs2 = 0., NN_mcp_fs1 = 0., NN_mcp_fs2 = 0., NN_ham_fs1 = 0., NN_ham_fs2 = 0.;
        Ndark_fs1 = 0., Ndark_fs2 = 0., Ndark_mcp_fs1 = 0., Ndark_mcp_fs2 = 0., Ndark_ham_fs1 = 0., Ndark_ham_fs2 = 0.;
        chi2_fs1 = 0., chi2_fs2 = 0., chi2_mcp_fs1 = 0., chi2_mcp_fs2 = 0., chi2_ham_fs1 = 0., chi2_ham_fs2 = 0.;
        ndf_fs1 = 0., ndf_fs2 = 0., ndf_mcp_fs1 = 0., ndf_mcp_fs2 = 0., ndf_ham_fs1 = 0., ndf_ham_fs2 = 0.;

        if (dr3 < dr3cut && Eqen_o > 10.) {

          //double xLow = 40.;
          //double xHigh = 800;
          //struct fitRes res1 = myroofit(treeAll_tem, timetest5_fs1, evtID_n, 0, 0, xHigh);
          //double ndf1 = GetNdf(treeAll_tem, 0, 40, xHigh);
          //tau1_fs1 = res1.tau1;
          //tau2_fs1 = res1.tau2;
          //// tau3_fs1 = res1.tau3;
          //eta1_fs1 = res1.tau1ratio;
          //// eta2_fs1 = res1.tau2ratio;
          //NN_fs1 = res1.nn0;
          //Ndark_fs1 = res1.ndark;
          //chi2_fs1 = res1.chi2;
          //ndf_fs1 = ndf1;

          double ndf2 = GetNdf(treeAll_tem, 0,40,800);
          struct fitRes res2 = myroofit(treeAll_tem, timetest5_fs2, evtID_n, 1, 1, 800);
          //struct fitRes res2 = myroofit(treeAll_tem, timetest5_fs2, evtID_n, 1, 1, xHigh);
          //double dsig_tau2 = res2.tau2err / res2.tau2;
          //double ndf2 = GetNdf(treeAll_tem, 1, 40, xHigh);

          //if (dsig_tau2 > 0.5) {
          //  int ibest = -1;
          //  double dsig_lowest = dsig_tau2;
          //  for (int ie = 0; ie < 11; ie++) {
          //    double ix = 600 + ie * 50;
          //    cout << "------------> ix = " << ix << endl;
          //    res2 = myroofit(treeAll_tem, timetest5_fs2, evtID_n, 1, 1, ix);
          //    double dsig_tau2_tem = res2.tau2err / res2.tau2;
          //    if (dsig_lowest > dsig_tau2_tem) {
          //      dsig_lowest = dsig_tau2_tem;
          //      ibest = ie;
          //    }
          //  }

          //  double ixbest = 600 + ibest * 50;
          //  res2 = myroofit(treeAll_tem, timetest5_fs2, evtID_n, 1, 1, ixbest);
          //  ndf2 = GetNdf(treeAll_tem, 1, 40, ixbest);
          //}
          tau1_fs2 = res2.tau1;
          tau2_fs2 = res2.tau2;
          // tau3_fs2 = res2.tau3;
          eta1_fs2 = res2.tau1ratio;
          // eta2_fs2 = res2.tau2ratio;
          NN_fs2 = res2.nn0;
          Ndark_fs2 = res2.ndark;
          chi2_fs2 = res2.chi2;
          ndf_fs2 = ndf2;
        }

        //---------------------for checking 20210917

        // TCanvas* ctest = new TCanvas();
        // gPad->SetLogy();
        // timeCheck01->Draw("E1");
        // TString ctestname = "ctest_";
        // ctestname += evtID_n;
        // ctestname += ".png";
        // ctest->SaveAs(ctestname);
        delete timeCheck01;
        // delete ctest;

        for (int ibin = 1; ibin <= 1000; ibin++) {
          double co3 = timetest1new_fs1->GetBinContent(ibin + timetag2);
          timetest3_fs1->SetBinContent(ibin, co3);

          co3 = timetest1new_fs2->GetBinContent(ibin + timetag3);
          timetest3_fs2->SetBinContent(ibin, co3);

          co3 = timetest1new_fsham1->GetBinContent(ibin + timetag4);
          timetest3_fsham1->SetBinContent(ibin, co3);

          co3 = timetest1new_fsham2->GetBinContent(ibin + timetag5);
          timetest3_fsham2->SetBinContent(ibin, co3);
        }

        int bin1_fs1 = timetest3_fs1->FindBin(0.);
        int bin2_fs1 = timetest3_fs1->FindBin(800.);
        timetest3_fs1->Scale(1. / timetest3_fs1->Integral(bin1_fs1, bin2_fs1));
        timetest3_fs2->Scale(1. / timetest3_fs2->Integral(bin1_fs1, bin2_fs1));

        int bin1_fsham1 = timetest3_fsham1->FindBin(-40.);
        int bin2_fsham1 = timetest3_fsham1->FindBin(800.);
        timetest3_fsham1->Scale(1. / timetest3_fsham1->Integral(bin1_fsham1, bin2_fsham1));
        timetest3_fsham2->Scale(1. / timetest3_fsham2->Integral(bin1_fsham1, bin2_fsham1));

        if (dr3 < 4096. && Eqen_o > 11. && Eqen_o < 31.) {
          pmtime_fs1_lowR_lowE->Add(timetest3_fs1);
          pmtime_fs2_lowR_lowE->Add(timetest3_fs2);
          pmtime_fsham1_lowR_lowE->Add(timetest2_fsham1);
          pmtime_fsham2_lowR_lowE->Add(timetest2_fsham2);
        }

        double rr = sqrt(Xrec_o * Xrec_o + Yrec_o * Yrec_o) * 1e-3;
        if (dr3 > 4096. && Eqen_o < 31. && Eqen_o > 11. && rr < 16. && Zrec_o < 16e3) {
          pmtime_fs1_highR_lowE->Add(timetest3_fs1);
          pmtime_fs2_highR_lowE->Add(timetest3_fs2);
          pmtime_fsham1_highR_lowE->Add(timetest2_fsham1);
          pmtime_fsham2_highR_lowE->Add(timetest2_fsham2);
        }

        if (dr3 < 4096. && Eqen_o < 60. && Eqen_o > 31.) {
          pmtime_fs1_lowR_highE->Add(timetest3_fs1);
          pmtime_fs2_lowR_highE->Add(timetest3_fs2);
          pmtime_fsham1_lowR_highE->Add(timetest2_fsham1);
          pmtime_fsham2_lowR_highE->Add(timetest2_fsham2);
        }

        if (dr3 > 4096. && Eqen_o < 60. && Eqen_o > 31. && rr < 16. && Zrec_o < 16e3) {
          pmtime_fs1_highR_highE->Add(timetest3_fs1);
          pmtime_fs2_highR_highE->Add(timetest3_fs2);
          pmtime_fsham1_highR_highE->Add(timetest2_fsham1);
          pmtime_fsham2_highR_highE->Add(timetest2_fsham2);
        }

        delete timetest1_fs1;
        delete timetest1new_fs1;
        delete timetest3_fs1;
        delete timetest4_fs1;
        delete timetest5_fs1;
        delete timetest1_fs2;
        delete timetest1new_fs2;
        delete timetest3_fs2;
        delete timetest4_fs2;
        delete timetest5_fs2;
        delete timetest1_fsmcp1;
        delete timetest4_fsmcp1;
        delete timetest5_fsmcp1;
        delete timetest1_fsmcp2;
        delete timetest4_fsmcp2;
        delete timetest5_fsmcp2;
        delete timetest4_fsham1;
        delete timetest5_fsham1;

        delete timetest1_fsham1;
        delete timetest1new_fsham1;
        delete timetest2_fsham1;
        delete timetest2new_fsham1;
        delete timetest3_fsham1;
        delete timetest1_fsham2;
        delete timetest2_fsham2;
        delete timetest1new_fsham2;
        delete timetest2new_fsham2;
        delete timetest3_fsham2;
        delete timetest4_fsham2;
        delete timetest5_fsham2;
        delete treeAll_tem;
        if (Eqen_o > 1.) {
          t_psd->Fill();
        }
      }
    }
    m_timeBuffer.clear();
  }

  m_elecIBDlikeID.clear();
  f_psd1->cd();
  pmtime_fs1_lowR_lowE->Write();
  pmtime_fs1_highR_lowE->Write();
  pmtime_fs1_lowR_highE->Write();
  pmtime_fs1_highR_highE->Write();
  pmtime_fs2_lowR_lowE->Write();
  pmtime_fs2_highR_lowE->Write();
  pmtime_fs2_lowR_highE->Write();
  pmtime_fs2_highR_highE->Write();

  pmtime_fsham1_lowR_lowE->Write();
  pmtime_fsham1_highR_lowE->Write();
  pmtime_fsham1_lowR_highE->Write();
  pmtime_fsham1_highR_highE->Write();
  pmtime_fsham2_lowR_lowE->Write();
  pmtime_fsham2_highR_lowE->Write();
  pmtime_fsham2_lowR_highE->Write();
  pmtime_fsham2_highR_highE->Write();
  f_psd1->Close();
  f_psd2->cd();
  t_psd->Write();
  f_psd2->Close();
}

void PMTPosition()
{
  TString filename = "/junofs/users/chengjie/workdir/PSD/Ana-new0817/input/PMTPos_Acrylic_with_chimney.csv";
  ifstream sfile;
  sfile.open(filename, ios::in);
  for (int i = 0; i < nPMTs; i++) {
    PMTpos_x[i] = 0.;
    PMTpos_y[i] = 0.;
    PMTpos_z[i] = 0.;
  }

  while (!sfile.eof()) {
    int tmp_copyno;
    double tmp_theta, tmp_phi;
    double tmp_x, tmp_y, tmp_z;

    sfile >> tmp_copyno >> tmp_x >> tmp_y >> tmp_z >> tmp_theta >> tmp_phi;

    if (!sfile.eof()) {
      PMTpos_x[tmp_copyno] = tmp_x;
      PMTpos_y[tmp_copyno] = tmp_y;
      PMTpos_z[tmp_copyno] = tmp_z;
    }
  }
  sfile.close();
}

void ReadPMTdata()
{
  TFile* pmtf = new TFile("/junofs/users/chengjie/workdir/PSD/Ana-new0817/input/PmtData.root");
  TTree* pmtdata = (TTree*)pmtf->Get("PmtData");
  int m_pmtId;
  int m_isHamamatsu;
  double m_darkRate;
  double m_timeSpread;
  double m_timeOffset;

  pmtdata->SetBranchAddress("pmtId", &m_pmtId);
  pmtdata->SetBranchAddress("isHamamatsu", &m_isHamamatsu);
  pmtdata->SetBranchAddress("darkRate", &m_darkRate);
  pmtdata->SetBranchAddress("timeSpread", &m_timeSpread);
  pmtdata->SetBranchAddress("timeOffset", &m_timeOffset);

  for (int i = 0; i < nPMTs2; i++) {
    darkRate[i] = 0.;
    timeSpread[i] = 0.;
    timeOffset[i] = 0.;
    isHamamatsu[i] = 0;
  }
  ndarkrate0 = 0.;
  ndarkrate1 = 0.;
  ndarkrate2 = 0.;

  for (int i = 0; i < nPMTs2; i++) {
    pmtdata->GetEntry(i);
    darkRate[m_pmtId] = m_darkRate;
    timeSpread[m_pmtId] = m_timeSpread;
    timeOffset[m_pmtId] = m_timeOffset;
    isHamamatsu[m_pmtId] = m_isHamamatsu;
    ndarkrate0 = m_darkRate + ndarkrate0;
    if (m_isHamamatsu == 0) {
      ndarkrate1 = m_darkRate + ndarkrate1;
    } else {
      ndarkrate2 = m_darkRate + ndarkrate2;
    }
  }
  pmtf->Close();
}
double GetTimetag(TH1F* h1tem)
{
  std::map<int, double> timecount1;
  timecount1.clear();
  for (int ibin = 1; ibin <= 1800; ibin++) {
    double co1 = h1tem->GetBinContent(ibin);
    int t_tem = ibin - 200;
    timecount1.insert(pair<int, double>(t_tem, co1));
  }
  vector<PAIR> vec_fs(timecount1.begin(), timecount1.end());
  sort(vec_fs.begin(), vec_fs.end(), cmp);
  double timetag = vec_fs[0].first;
  return timetag;
}

double GetPeakvalue(TH1F* h1tem)
{
  std::map<int, double> timecount1;
  timecount1.clear();
  for (int ibin = 1; ibin <= 1800; ibin++) {
    double co1 = h1tem->GetBinContent(ibin);
    int t_tem = ibin - 200;
    timecount1.insert(pair<int, double>(t_tem, co1));
  }
  vector<PAIR> vec_fs(timecount1.begin(), timecount1.end());
  sort(vec_fs.begin(), vec_fs.end(), cmp);
  double peak = vec_fs[0].second;
  return peak;
}

double GetTail2total(TH1F* h1tem, double ttag, int istep)
{
  double ntot = 0.;
  double ntail = 0.;
  double rpsd = 0.;
  double startt = 10.;
  double tailcut = 0.;
  int tagb0 = h1tem->FindBin(ttag);
  int tagb1 = h1tem->FindBin(ttag + 800);
  ntot = h1tem->Integral(tagb0, tagb1);
  tailcut = startt + istep * 10.;
  double ttag2 = ttag + tailcut;
  int tagb2 = h1tem->FindBin(ttag2);
  ntail = h1tem->Integral(tagb2, tagb1);
  rpsd = ntail / ntot;
  return rpsd;
}

double GetW1new(TH1F* h1tem, double cut)
{
  double binstart = h1tem->FindBin(0.);
  double W1new = 0;
  double yy1 = 0;
  double yy2 = 0;

  for (int wt = binstart; wt < binstart + 10; wt++) {
    double tm1 = 0;
    double tm2 = 0;
    tm1 = h1tem->GetBinContent(wt);
    tm2 = h1tem->GetBinContent(wt + 1);
    yy1 = tm1;
    yy2 = tm2;
    if (yy1 > cut && yy2 <= cut) {
      double xx1 = wt - binstart;
      double xx2 = xx1 + 1;
      double aa = (yy2 - yy1);
      double bb = xx2 * yy1 - xx1 * yy2;
      W1new = (cut - bb) / aa;
      break;
    }
  }
  return W1new;
}

double GetW2new(TH1F* h1tem, double cut)
{
  double binstart = h1tem->FindBin(0.);
  double W2new = 0;
  double yy1 = 0;
  double yy2 = 0;
  for (int wt = binstart; wt > binstart - 10; wt--) {
    double tm1 = 0;
    double tm2 = 0;
    tm1 = h1tem->GetBinContent(wt);
    tm2 = h1tem->GetBinContent(wt - 1);
    yy1 = tm1;
    yy2 = tm2;
    if (yy1 > cut && yy2 <= cut) {
      double xx1 = -1 * (wt - binstart);
      double xx2 = xx1 + 1;
      double aa = (yy2 - yy1);
      double bb = xx2 * yy1 - xx1 * yy2;
      W2new = (cut - bb) / aa;
      break;
    }
  }
  return W2new;
}

double GetW1(TH1F* h1tem, int ss)
{
  double tm1 = 0.;
  double W1 = 0.;
  for (int i = 1; i <= 4000; i++) {
    tm1 = h1tem->GetBinContent(i) + tm1;
    int tm2 = int(tm1 * 10);
    if (tm2 == ss + 1) {
      W1 = i * 0.01;
      break;
    }
  }
  return W1;
}

double GetW2(TH1F* h1tem, int ss)
{
  double tm1 = 0.;
  double W2 = 0.;
  for (int i = 4000; i >= 1; i--) {
    tm1 = h1tem->GetBinContent(i) + tm1;
    int tm2 = int(tm1 * 10);
    if (tm2 == ss + 1) {
      W2 = (4001 - i) * 0.01;
      break;
    }
  }
  return W2;
}

double GetNdf(TTree* ttree, int isWeight, double xlow, double xhigh)
{

  double ttnew;
  double nchargenew;

  ttree->SetBranchAddress("tt_0", &ttnew);
  ttree->SetBranchAddress("ncharge", &nchargenew);

  int nnew = int(ttree->GetEntries());
  double ndfnew = 0.;
  for (int jj = 0; jj < nnew; jj++) {
    ttree->GetEntry(jj);
    if (ttnew >= xlow && ttnew <= xhigh) {
      if (isWeight == 1) {
        ndfnew = ndfnew + nchargenew;
      } else {
        ndfnew = ndfnew + 1;
      }
    }
  }
  ndfnew = ndfnew - 4;
  return ndfnew;
}

struct fitRes myroofit(TTree* ttree, TH1F* h2, int evttag, int fittag, int isWeight, double xhigh)
{
    bool debug_plot = true;

  gSystem->Load("libRooFit");
  RooRealVar tt_0("tt_0", "time [ns]", 40., xhigh, "");
  RooRealVar ncharge("ncharge", "Q", 0., 100., "");
  // RooRealVar tau1("#tau_{1}", "tau1", 20., 0., 100.);
  RooRealVar tau1("#tau_{1}", "tau1", 20., 0., 800.);
  // RooRealVar tau2("#tau_{2}", "tau2", 150., 50., 500.);
  RooRealVar tau2("#tau_{2}", "tau2", 150., 0., 800.);
  // RooRealVar tau3("#tau_{3}", "tau3", 400, 200., 800.);
  RooRealVar eta1("#eta_{1}", "eta1", 0.7, 0., 1.);
  // RooRealVar eta2("#eta_{2}", "eta2", 0.2, 0., 1.);
  RooRealVar Nhit("N_{hit}", "Nhit", 10000, 1.e3, 1.e6);
  RooRealVar Ndark("N_{dark}", "Ndark", 0);
  // RooRealVar Ndark("N_{dark}", "Ndark", 0.5, 0.1, 10.);

  RooFormulaVar lambda1("lambda1", "lambda1", "-1./@0", RooArgList(tau1));
  RooFormulaVar lambda2("lambda2", "lambda2", "-1./@0", RooArgList(tau2));
  // RooFormulaVar lambda3("lambda3", "lambda3", "-1./@0", RooArgList(tau3));

  RooExponential exp1("exp1", "exp1 distribution", tt_0, lambda1);
  RooExponential exp2("exp2", "exp2 distribution", tt_0, lambda2);
  // RooExponential exp3("exp3", "exp3 distribution", tt_0, lambda3);

  RooFormulaVar bpN1("bpN1", "bpN1", "@0*@1", RooArgList(Nhit, eta1));
  RooFormulaVar bpN2("bpN2", "bpN2", "@0*(1.-@1)", RooArgList(Nhit, eta1));
  // RooFormulaVar bpN3("bpN3", "bpN3", "@0*(1.-@1-@2)", RooArgList(Nhit, eta1, eta2));
  RooFormulaVar bpdark("bpdark", "bpdark", "@0*(900.-40.)", RooArgList(Ndark));

  RooPolynomial polybkg1("polybkg1", "bkg1 distribution", tt_0, RooArgList());

  TF1* ffpoly = new TF1("ffpoly", "[0]", -200, -100);
  h2->Fit(ffpoly, "RL");
  double ndarkfit = ffpoly->GetParameter(0) * (xhigh - 40);
  double ndarkerrfit = ffpoly->GetParError(0) * (xhigh - 40);
  Ndark.setVal(ndarkfit);
  Ndark.setError(ndarkerrfit);


  if (debug_plot)
  {
    TCanvas* c2 = new TCanvas();
    gPad->SetLogy();
    h2->Draw("E1");
    ffpoly->SetLineColor(2);
    ffpoly->Draw("same");
    TString c2name = "c2_";
    c2name += evttag;
    c2name += "_in_anafs_";
    c2name += fittag;
    c2name += ".png";
    c2->SaveAs(c2name);
    delete c2;
  }

  RooAddPdf* sum1;
  // sum1 = new RooAddPdf("sum1", "sum1", RooArgList(exp1, exp2, exp3, polybkg1), RooArgList(bpN1, bpN2, bpN3, Ndark));
  sum1 = new RooAddPdf("sum1", "sum1", RooArgList(exp1, exp2, polybkg1), RooArgList(bpN1, bpN2, Ndark));
  // sum1 = new RooAddPdf("sum1", "sum1", RooArgList(exp1, exp2, exp3, polybkg1), RooArgList(bpN1, bpN2, bpN3, bpdark));
  RooDataSet* data;
  if (isWeight == 1) {
    cout << "------------------> test 20210917" << endl;
    data = new RooDataSet("data", "data", RooArgSet(tt_0, ncharge), Import(*ttree), WeightVar("ncharge"));
  } else {
    data = new RooDataSet("data", "data", RooArgSet(tt_0), Import(*ttree));
  }

  RooFitResult* fitresult;
  fitresult = sum1->fitTo(*data, Save(), PrintLevel(-1), Extended(kTRUE));
  fitresult->Print();

  std::map<double, double> taumap;
  taumap.clear();
  double eta2 = 1. - eta1.getVal(0);
  taumap.insert(pair<double, double>(tau1.getVal(0), eta1.getVal(0)));
  taumap.insert(pair<double, double>(tau2.getVal(0), eta2));
  vector<PAIR02> vec(taumap.begin(), taumap.end());
  sort(vec.begin(), vec.end(), cmp02);

  struct fitRes fs;
  fs.tau1 = vec[0].first;
  fs.tau2 = vec[1].first;
  fs.tau1ratio = vec[0].second;
  if (vec[0].first == tau1.getVal(0)) {
    fs.tau1err = tau1.getError();
    fs.tau2err = tau2.getError();
    fs.tau1ratioerr = eta1.getError();
  } else {
    fs.tau1err = tau2.getError();
    fs.tau2err = tau1.getError();
    fs.tau1ratioerr = 0;
  }
  // fs.tau2ratio = eta2.getVal(0);
  fs.ndark = Ndark.getVal(0);
  fs.nn0 = Nhit.getVal(0);
  fs.chi2 = fitresult->minNll();
  fs.ndf = 0;

  if (debug_plot)
  {
   TCanvas* c1 = new TCanvas("c1");
   //gPad->SetLogy();
  // gPad->Range(40,0.1, 1000, 1e3);

   RooPlot* xframe = tt_0.frame();
  // xframe->GetYaxis()->SetRangeUser(1e-2, 1e3);
  // gPad->SetLogy();
   data->plotOn(xframe);
   sum1->plotOn(xframe, Components(RooArgSet(exp1)), LineStyle(1), LineColor(kRed));
   sum1->plotOn(xframe, Components(RooArgSet(exp2)), LineStyle(kDashed), LineColor(kBlue));
  // sum1->plotOn(xframe, Components(RooArgSet(exp3)), LineStyle(kDashed), LineColor(kOrange + 2));
   sum1->plotOn(xframe, Components(RooArgSet(polybkg1)), LineStyle(kDotted), LineColor(kGreen));
   sum1->plotOn(xframe, LineStyle(1), LineColor(1));
   xframe->Draw();
   TString c1name = "c1_";
   c1name += evttag;
   c1name += "_in_anafs_";
   c1name += fittag;
   c1name += ".png";
   c1->SaveAs(c1name);
   delete c1;
  }

  return fs;
}

