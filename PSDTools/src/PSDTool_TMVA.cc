#include "PSDTools/PSDTool_TMVA.h"

#include "SniperKernel/ToolFactory.h"
#include "SniperKernel/SniperPtr.h"
#include "RootWriter/RootWriter.h"

#include <TSystem.h>
#include <iostream>

#include "TH1.h"
#include "TF1.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TString.h"
#include <string>

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

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/MethodCuts.h"

using namespace RooFit;
using namespace std;

typedef pair<double, double> PAIR02;
int cmp03(const PAIR02& x, const PAIR02& y)  //
{
  return x.first < y.first;
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

double GetTail2total(TH1F* h1tem, double ttag, int istep)
{
  double ntot = 0.;
  double ntail = 0.;
  double rpsd = 0.;
  double startt = 10.;
  double tailcut = 0.;
  int tagb0 = h1tem->FindBin(ttag);
  int tagb1 = h1tem->FindBin(ttag + 1300);
  ntot = h1tem->Integral(tagb0, tagb1);
  tailcut = startt + istep * 10.;
  double ttag2 = ttag + tailcut;
  int tagb2 = h1tem->FindBin(ttag2);
  ntail = h1tem->Integral(tagb2, tagb1);
  rpsd = ntail / ntot;
  return rpsd;
}

struct fitRes PSD_TMVA::FitToGetTimeConstants(const int fittag = 0, const double xhigh = 1100.)
// tree_time_falling_edge is for the falling edge t~[0,1000] and h_time_rising_edge is for the rising edge t~[-200,0]
{

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
  h_time_rising_edge->Fit(ffpoly, "RL");
  double ndarkfit = ffpoly->GetParameter(0) * (xhigh - 40);
  double ndarkerrfit = ffpoly->GetParError(0) * (xhigh - 40);
  Ndark.setVal(ndarkfit);
  Ndark.setError(ndarkerrfit);

  if (debug_plot_fit_result) {
    // Debug
    TString name_c2 = "c_fitting_rising_edge";
    TCanvas* c2 = new TCanvas(name_c2 + "_" + evtID);
    gPad->SetLogy();
    h_time_rising_edge->Draw("E1");
    ffpoly->SetLineColor(2);
    ffpoly->Draw("same");
    TString c2name = "c2_";
    c2name += evtID;
    c2name += "_";
    c2name += fittag;
    c2name += ".png";
    c2->SaveAs(c2name);
    delete c2;
  }

  RooAddPdf* sum1;
  sum1 = new RooAddPdf("sum1", "sum1", RooArgList(exp1, exp2, polybkg1), RooArgList(bpN1, bpN2, Ndark));
  RooDataSet* data;

  if (whether_with_charge) {
    data = new RooDataSet("data", "data", RooArgSet(tt_0, ncharge), Import(*tree_time_falling_edge), WeightVar("ncharge"));
  } else {
    data = new RooDataSet("data", "data", RooArgSet(tt_0), Import(*tree_time_falling_edge));
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
  sort(vec.begin(), vec.end(), cmp03);

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

  if (debug_plot_fit_result) {
    // Debug Plotting
    TString name_c1 = "c_fitting_falling_edge";
    TCanvas* c1 = new TCanvas(name_c1 + "_" + evtID);
    // gPad->SetLogy();

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
    c1name += evtID;
    c1name += "_";
    c1name += fittag;
    c1name += ".png";
    c1->SaveAs(c1name);
    delete c1;
  }

  return fs;
}

DECLARE_TOOL(PSD_TMVA);

PSD_TMVA::PSD_TMVA(const std::string& name) : ToolBase(name)
{
    // For two models
//    declProp("Model_FV1", model_FV1);
//    declProp("Model_FV2", model_FV2);
    declProp("Model", model);
//    declProp("R_divide", R_divide);
    declProp("PSD_divide", PSD_divide);
}

PSD_TMVA::~PSD_TMVA()
{
}

bool PSD_TMVA::initialize()
{
  LogInfo << "Initializing PSD_TMVA..." << std::endl;

  debug_plot_fit_result = false;

  // get the rootwriter
  SniperPtr<RootWriter> rwsvc(getParent(), "RootWriter");
  if (rwsvc.invalid()) {
    LogError << "cannot get the rootwriter service." << std::endl;
    return false;
  }

  evtID = 0;

  m_constant_to_save = {0, 0, 0, 0, 0, 0, 0};
  m_w1 = 0;
  m_w2 = 0;
  m_vertex_r3_of_event = 0;
  m_E_rec = 0;

  h_time_falling_edge = new TH1F("h_time_falling_edge", "h_time_falling_edge", 800, 0, 800);
  h_time_rising_edge = new TH1F("h_time_rising_edge", "h_time_rising_edge", 200, -200, 0);
  // only Hamamtsu PMTs
  h_time_around_peak_ham = new TH1F("h_time_around_peak_ham", "h_time_around_peak_ham", 4000, -20, 20);
  h_time = new TH1F("h_time", "h_time", 2000, -200, 1800);
  // only Hamamtsu PMTs ------- added by Jie in 2021/11/26
  h_time_ham = new TH1F("h_time_ham", "h_time_ham", 2000, -200, 1800);
  //

  // tree_time_falling_edge = new TTree("tree_time_falling_edge", "");
  tree_time_falling_edge = rwsvc->bookTree(*m_par,"tree_time_falling_edge", "");
  tree_time_falling_edge->Branch("tt_0", &tt, "tt_0/D");
  tree_time_falling_edge->Branch("ncharge", &charge, "ncharge/D");
  // rwsvc->attach("USER_OUTPUT", tree_time_falling_edge);

  // Store the pre-processed events
  gROOT->ProcessLine("#include <vector>");
  m_userTree = rwsvc->bookTree(*m_par,"evt", "evt");
  m_userTree->Branch("evtID", &evtID, "evtID/I");
  m_userTree->Branch("tau1", &m_constant_to_save.tau1, "tau1/F");
  m_userTree->Branch("tau2", &m_constant_to_save.tau2, "tau2/F");
  m_userTree->Branch("eta1", &m_constant_to_save.tau1ratio, "eta1/F");
  m_userTree->Branch("ndark", &m_constant_to_save.ndark, "ndark/F");
  m_userTree->Branch("nn0", &m_constant_to_save.nn0, "nn0/F");
  m_userTree->Branch("chi2", &m_constant_to_save.chi2, "chi2/F");
  m_userTree->Branch("w1", &m_w1, "w1/F");
  m_userTree->Branch("w2", &m_w2, "w2/F");
  m_userTree->Branch("R_peak_to_tail", &m_peak_to_tail_ratio, "R_peak_to_tail/F");
  m_userTree->Branch("R_tail_to_tail", &m_tail_to_tail_ratio, "R_tail_to_tail/F");
  //----- added by Jie in 2021/11/26
  m_userTree->Branch("R_peak_to_total_ham", &m_peak_to_total_ratio_ham, "R_peak_to_total_ham/F");
  m_userTree->Branch("R_tail_to_total_ham", &m_tail_to_total_ratio_ham, "R_tail_to_total_ham/F");
  //

  m_userTree->Branch("r3_tag", &m_vertex_r3_of_event, "r3_tag/F");  // (x^2+y^2+z^2)^3/2
  m_userTree->Branch("recE", &m_E_rec, "recE/F");
  //    m_userTree->Branch("h_time_w", &v_h_time_w );

  rwsvc->attach("USER_OUTPUT", m_userTree);

  // Store the PSDTools result, which may be implemented in data model in the future
  m_psdTree = rwsvc->bookTree(*m_par,"PSDTools", "PSDTools");
  m_psdTree->Branch("evtID", &evtID, "evtID/I");
  m_psdTree->Branch("psdVal", &m_psdEvent.psdVal, "psdVal/D");
  m_psdTree->Branch("evtType", &m_psdEvent.evtType, "evtType/I");
  m_psdTree->Branch("r3_tag", &m_vertex_r3_of_event, "r3_tag/F");  // (x^2+y^2+z^2)^3/2
  m_psdTree->Branch("E_rec", &m_E_rec, "E_rec/F");

  // rwsvc->attach("PSD_OUTPUT", m_psdTree);
  rwsvc->attach("USER_OUTPUT", m_psdTree);

  m_psdInput = dynamic_cast<IPSDInputSvc*>(getParent()->find("PSDInputSvc"));
  whether_with_charge = true;

  if (b_usePredict) {
    TMVA::Tools::Instance();
    std::string ReaderCoef_BDTG = "!Color:!Silent";
    reader_BDTG = new TMVA::Reader(ReaderCoef_BDTG.c_str());
    reader_BDTG->AddVariable("w1", &m_w1);
    reader_BDTG->AddVariable("w2", &m_w2);
    reader_BDTG->AddVariable("tau1", &m_constant_to_save.tau1);
    reader_BDTG->AddVariable("tau2", &m_constant_to_save.tau2);
    reader_BDTG->AddVariable("eta1", &m_constant_to_save.tau1ratio);
    reader_BDTG->AddVariable("ndark", &m_constant_to_save.ndark);
    reader_BDTG->AddVariable("rpsdpeak_ham", &m_peak_to_total_ratio_ham);
    reader_BDTG->AddVariable("rpsdtail_ham", &m_tail_to_total_ratio_ham);
    reader_BDTG->AddVariable("r3", &m_vertex_r3_of_event);
    ///////////////// More standard code /////////////////////////
//    reader_BDTG->AddVariable("R_peak_to_tail", &m_peak_to_tail_ratio);
//    reader_BDTG->AddVariable("R_tail_to_tail", &m_tail_to_tail_ratio);
//    reader_BDTG->AddVariable("R_peak_to_total_ham", &m_peak_to_total_ratio_ham);
//    reader_BDTG->AddVariable("R_tail_to_total_ham", &m_tail_to_total_ratio_ham);
//    reader_BDTG->AddVariable("r3_tag", &m_vertex_r3_of_event);
////////////////////////////////////////////////////////////////////

    reader_BDTG->AddSpectator("Eqe", &Equench);
    reader_BDTG->AddSpectator("id_tag", &id_tag);
    reader_BDTG->AddSpectator("isoz", &isoz);
    reader_BDTG->AddSpectator("ison", &ison);

    reader_BDTG->BookMVA("BDTG", model);
    // For two model
//    reader_BDTG->BookMVA("BDTG FV1", model_FV1);
//    reader_BDTG->BookMVA("BDTG FV2", model_FV2);
  }

  return true;
}

bool PSD_TMVA::finalize()
{
  LogInfo << "Finalizing PSD_TMVA..." << std::endl;
  return true;
}

bool PSD_TMVA::preProcess(JM::EvtNavigator* nav)
{
  LogDebug << "pre processing an event..." << std::endl;
  if (!m_psdInput->extractHitInfo(nav, "alignPeak2")) return false;

  // Reset histograms' bins content
  //   v_h_time_w.clear();
  h_time_rising_edge->Reset();
  h_time_falling_edge->Reset();
  tree_time_falling_edge->Reset();
  h_time_around_peak_ham->Reset();
  h_time->Reset();
  h_time_ham->Reset();

  vector<double> v_hittime = m_psdInput->getHitTime();
  vector<double> v_charge = m_psdInput->getHitCharge();
  vector<int> v_isHama = m_psdInput->getHitIsHama();

  for (unsigned long int i = 0; i < v_hittime.size(); i++) {
    tt = v_hittime[i];
    charge = 1;
    if (whether_with_charge) charge = v_charge[i];

    tree_time_falling_edge->Fill();

    if (tt >= 0. && tt < 1000) {
      h_time_falling_edge->Fill(tt, charge);
    }
    if (tt >= -200. && tt < 0) {
      h_time_rising_edge->Fill(tt, charge);
    }

    h_time->Fill(tt, charge);

    // Only fill hits of Hamamtsu PMTs
    if (v_isHama[i] && tt > -20. && tt < 20.) h_time_around_peak_ham->Fill(tt, charge);
    if (v_isHama[i]) h_time_ham->Fill(tt, charge);
  }

  m_constant_to_save = FitToGetTimeConstants(0);

  int ss = 0;  // option for function GetW*(), which means only get integral area=0.1
  h_time_around_peak_ham->Scale(1. / h_time_around_peak_ham->Integral());

  //    for (int i=0;i<h_time_around_peak_ham->GetNbinsX();i++)
  //        v_h_time_w.push_back(h_time_around_peak_ham->GetBinContent(i));
  // Get parameters about peak ( w1 and w2 )
  m_w1 = GetW1(h_time_around_peak_ham, ss);
  m_w2 = GetW2(h_time_around_peak_ham, ss);

  // Get parameters for TTR
  m_peak_to_tail_ratio = GetTail2total(h_time, 0, 1);
  m_tail_to_tail_ratio = GetTail2total(h_time, 0, 39);

  m_peak_to_total_ratio_ham = 0.;
  m_tail_to_total_ratio_ham = 0.;
  m_peak_to_total_ratio_ham = GetTail2total(h_time_ham, 0, 1);
  m_tail_to_total_ratio_ham = GetTail2total(h_time_ham, 0, 39);




  // Debug

  if (debug_plot_fit_result) {
    auto c1 = new TCanvas("c_rising", "c_rising", 800, 600);
    h_time_rising_edge->Draw("hist");
    auto c2 = new TCanvas("c_falling", "c_falling", 800, 600);
    h_time_falling_edge->Draw("hist");
    auto c3 = new TCanvas("c_h_time", "c_h_time", 800, 600);
    h_time->Draw("hist");
    c3->SetLogy();
    c1->SaveAs((TString)c1->GetTitle() + "_" + evtID + ".png");
    c2->SaveAs((TString)c2->GetTitle() + "_" + evtID + ".png");
    c3->SaveAs((TString)c3->GetTitle() + "_" + evtID + ".png");
    delete c1;
    delete c2;
    delete c3;
  }

  //------------calculate w1 and w2

  if (debug_plot_fit_result) {
    auto c1 = new TCanvas("c_around_peak", "c_around_peak", 800, 600);
    h_time_around_peak_ham->Draw("hist");
    c1->SaveAs((TString)c1->GetTitle() + "_" + evtID + ".png");
  }

  // -------Get Event Vertex-------------
  m_vertex_r3_of_event = m_psdInput->getVertexR3();
  m_E_rec = m_psdInput->getErec();

  // Only when disable Predict mode is on, we will save the fitting outputs so that we can do the training in the next step
  if (!b_usePredict) {
    m_userTree->Fill();
    evtID++;
  }

  return true;
}

double PSD_TMVA::CalPSDVal()
{
  cout << "-------Try to Predict----" << endl;
  ////////////////////// For two models ///////////////////////////////////////
  //    cout<< "model FV1:\t"<<(double)(reader_BDTG->EvaluateMVA("BDTG FV1"))<<endl;
  //    cout<< "model FV2:\t"<< (double)(reader_BDTG->EvaluateMVA("BDTG FV2"))<<endl;
//  if (m_vertex_r3_of_event <= pow(R_divide, 3))
//    m_psdEvent.psdVal = (double)(reader_BDTG->EvaluateMVA("BDTG FV1"));
//  else
//    m_psdEvent.psdVal = (double)(reader_BDTG->EvaluateMVA("BDTG FV2"));
  ////////////////////////////////////////////////////////////////////////////////

  m_psdEvent.psdVal = (double)(reader_BDTG->EvaluateMVA("BDTG"));

  cout << "BDTG Output:\t" << m_psdEvent.psdVal << endl;
  cout << "--------------------------" << endl;

  // Set default tag for prediction output
  if (m_psdEvent.psdVal >= PSD_divide)
    m_psdEvent.evtType = EVTTYPE::Electron;
  else
    m_psdEvent.evtType = EVTTYPE::Proton;

  m_psdTree->Fill();
  evtID++;
  return m_psdEvent.psdVal;
}

