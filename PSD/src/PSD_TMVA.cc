#include "PSD_TMVA.h"

#include "SniperKernel/ToolBase.h"
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
#include <string>

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"


using namespace RooFit;
using namespace std;

typedef pair<double, double> PAIR02;
int cmp03(const PAIR02& x, const PAIR02& y)  //
{
  return x.first < y.first;
}

float GetW1(TH1F* h1tem, int ss)
{
  double tm1 = 0.;
  float W1 = 0.;
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

float GetW2(TH1F* h1tem, int ss)
{
  double tm1 = 0.;
  float W2 = 0.;
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


struct fitRes PSD_TMVA::FitToGetTimeConstants( int fittag=0)
// tree_time_falling_edge is for the falling edge t~[0,1000] and h_time_rising_edge is for the rising edge t~[-200,0]
{

  gSystem->Load("libRooFit");
  RooRealVar tt_0("tt_0", "time [ns]", 40., 800., "");
  RooRealVar ncharge("ncharge", "Q", 0., 100., "");
  RooRealVar tau1("#tau_{1}", "tau1", 20., 0., 800.);
  RooRealVar tau2("#tau_{2}", "tau2", 150., 0., 800.);
  RooRealVar eta1("#eta_{1}", "eta1", 0.7, 0., 1.);
  RooRealVar Nhit("N_{hit}", "Nhit", 10000, 1.e3, 1.e6);
  RooRealVar Ndark("N_{dark}", "Ndark", 0);

  RooFormulaVar lambda1("lambda1", "lambda1", "-1./@0", RooArgList(tau1));
  RooFormulaVar lambda2("lambda2", "lambda2", "-1./@0", RooArgList(tau2));

  RooExponential exp1("exp1", "exp1 distribution", tt_0, lambda1);
  RooExponential exp2("exp2", "exp2 distribution", tt_0, lambda2);

  RooFormulaVar bpN1("bpN1", "bpN1", "@0*@1", RooArgList(Nhit, eta1));
  RooFormulaVar bpN2("bpN2", "bpN2", "@0*(1.-@1)", RooArgList(Nhit, eta1));
  RooFormulaVar bpdark("bpdark", "bpdark", "@0*(800.-40.)", RooArgList(Ndark));

  RooPolynomial polybkg1("polybkg1", "bkg1 distribution", tt_0, RooArgList());

  TF1* ffpoly = new TF1("ffpoly", "[0]", -200, -100);
  h_time_rising_edge->Fit(ffpoly, "RL");
  double ndarkfit = ffpoly->GetParameter(0) * (800 - 40);
  double ndarkerrfit = ffpoly->GetParError(0) * (800 - 40);
  Ndark.setVal(ndarkfit);
  Ndark.setError(ndarkerrfit);

  if (debug_plot_fit_result)
  {
    // Debug
    TString name_c2 = "c_fitting_rising_edge";
    TCanvas* c2 = new TCanvas(name_c2+"_"+evtID);
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
  // fs.tau3 = tau3.getVal(0);
  fs.tau1ratio = vec[0].second;
  // fs.tau2ratio = eta2.getVal(0);
  fs.ndark = Ndark.getVal(0);
  fs.nn0 = Nhit.getVal(0);
  fs.chi2 = fitresult->minNll();
  fs.ndf = 0;

  if (debug_plot_fit_result)
  {
    // Debug Plotting 
    TString name_c1 = "c_fitting_falling_edge";
    TCanvas* c1 = new TCanvas(name_c1+"_"+evtID);
    //gPad->SetLogy();
    
    // gPad->Range(40,0.1, 1000, 1e3);

    RooPlot* xframe = tt_0.frame();
    //xframe->GetYaxis()->SetRangeUser(1e-2, 1e3);
    //gPad->SetLogy();
    data->plotOn(xframe);
    sum1->plotOn(xframe, Components(RooArgSet(exp1)), LineStyle(1), LineColor(kRed));
    sum1->plotOn(xframe, Components(RooArgSet(exp2)), LineStyle(kDashed), LineColor(kBlue));
    //sum1->plotOn(xframe, Components(RooArgSet(exp3)), LineStyle(kDashed), LineColor(kOrange + 2));
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

PSD_TMVA::PSD_TMVA(const std::string &name): ToolBase(name){
}

PSD_TMVA::~PSD_TMVA(){
}

bool PSD_TMVA::initialize(){
    LogInfo<<"Initializing PSD_TMVA..."<<std::endl;

    debug_plot_fit_result = false;

    //get the rootwriter
    SniperPtr<RootWriter> rwsvc(getParent(),"RootWriter");
    if ( rwsvc.invalid() ) {
        LogError << "cannot get the rootwriter service." << std::endl;
        return false;
    }
 
    evtID = 0;

    m_constant_to_save = {0, 0, 0, 0, 0, 0, 0};
    m_w1 = 0;
    m_w2 = 0;
    m_vertex_r3_of_event = 0;

    h_time_falling_edge = new TH1F("h_time_falling_edge", "h_time_falling_edge", 800, 0, 800);
    h_time_rising_edge = new TH1F("h_time_rising_edge", "h_time_rising_edge", 200, -200, 0);
    // only Hamamtsu PMTs 
    h_time_around_peak_ham = new TH1F("h_time_around_peak_ham", "h_time_around_peak_ham", 4000, -20, 20);

    //tree_time_falling_edge = new TTree("tree_time_falling_edge", "");
    tree_time_falling_edge = rwsvc->bookTree("tree_time_falling_edge", "");
    tree_time_falling_edge->Branch("tt_0", &tt, "tt_0/D");
    tree_time_falling_edge->Branch("ncharge", &charge, "ncharge/D");
    //rwsvc->attach("USER_OUTPUT", tree_time_falling_edge);
 
    //Store the pre-processed events
    m_userTree=rwsvc->bookTree("evt", "evt");
    m_userTree->Branch("evtID", &evtID, "evtID/I");
    m_userTree->Branch("tau1", &m_constant_to_save.tau1, "tau1/F");
    m_userTree->Branch("tau2", &m_constant_to_save.tau2, "tau2/F");
    m_userTree->Branch("eta1" , &m_constant_to_save.tau1ratio, "eta1/F");
    m_userTree->Branch("ndark",&m_constant_to_save.ndark, "ndark/F");
    m_userTree->Branch("nn0",  &m_constant_to_save.nn0, "nn0/F");
    m_userTree->Branch("chi2", &m_constant_to_save.chi2, "chi2/F");
    m_userTree->Branch("w1"  , &m_w1 , "w1/F");
    m_userTree->Branch("w2"  , &m_w2 , "w2/F");
    m_userTree->Branch("r3_tag",&m_vertex_r3_of_event, "r3_tag/F"); // (x^2+y^2+z^2)^3/2

    rwsvc->attach("USER_OUTPUT", m_userTree);

    //Store the PSD result, which may be implemented in data model in the future
    m_psdTree=rwsvc->bookTree("PSD", "PSD");
    m_psdTree->Branch("evtID", &evtID, "evtID/I");
    m_psdTree->Branch("psdVal", &m_psdEvent.psdVal, "psdVal/D");
    m_psdTree->Branch("evtType", &m_psdEvent.evtType, "evtType/I");
    m_psdTree->Branch("r3_tag", &m_vertex_r3_of_event, "r3_tag/F"); // (x^2+y^2+z^2)^3/2
    m_psdTree->Branch("E_rec", &m_E_rec, "E_rec/F");

    // rwsvc->attach("PSD_OUTPUT", m_psdTree);
    rwsvc->attach("USER_OUTPUT", m_psdTree);

    m_psdInput = new PSDInput();
    whether_with_charge = true;
    
    if (b_usePredict)
    {
        cout<< "Loading model:\t"<< b_model << endl;
        std::string Method_str_BDT = b_model;
	    TMVA::Tools::Instance();
	    std::string ReaderCoef_BDTG="!Color:!Silent";
	    reader_BDTG  = new TMVA::Reader( ReaderCoef_BDTG.c_str() );
        reader_BDTG->AddVariable("w1", &m_w1);
        reader_BDTG->AddVariable("w2", &m_w2);
        reader_BDTG->AddVariable("tau1", &m_constant_to_save.tau1);
        reader_BDTG->AddVariable("tau2", &m_constant_to_save.tau2);
        reader_BDTG->AddVariable("eta1",  &m_constant_to_save.tau1ratio);
        reader_BDTG->AddVariable("r3_tag", &m_vertex_r3_of_event);

        reader_BDTG->AddSpectator("Eqe_tag", &Equench);
        reader_BDTG->AddSpectator("nPE_tag", &nPE_tag);
        reader_BDTG->AddSpectator("vtag", &v_tag);
        reader_BDTG->AddSpectator("id_tag", &id_tag);
        reader_BDTG->AddSpectator("isoz", &isoz);
        reader_BDTG->AddSpectator("ison", &ison);
	    reader_BDTG->BookMVA("BDTG method",Method_str_BDT);

    }

    return true;
}

bool PSD_TMVA::finalize(){
    LogInfo<<"Finalizing PSD_TMVA..."<<std::endl;
    delete m_psdInput;
    return true;
}

bool PSD_TMVA::preProcess(JM::EvtNavigator *nav){
    LogDebug<<"pre processing an event..."<<std::endl;
    if (!m_psdInput->extractHitInfo(nav)) return false;

   // Reset histograms' bins content 
   h_time_rising_edge->Reset();
   h_time_falling_edge->Reset();
   tree_time_falling_edge->Reset();
   h_time_around_peak_ham->Reset();
   
   m_psdInput->alignTime("alignPeak2");
   vector<double> v_hittime = m_psdInput->getHitTime();
   vector<double> v_charge =  m_psdInput->getHitCharge();
   vector<int   > v_isHama =  m_psdInput->getHitIsHama();
   
    for (unsigned long int i=0;i<v_hittime.size();i++)
    {
        tt = v_hittime[i];
        charge = 1;
        if (whether_with_charge) charge = v_charge[i];

        if (tt >= 0. && tt < 1000) {
          h_time_falling_edge->Fill(tt, charge);
          tree_time_falling_edge->Fill();
        }
        if (tt >= -200. && tt < 0) {
          h_time_rising_edge->Fill(tt, charge);
        }
        
        // Only fill hits of Hamamtsu PMTs
        if (v_isHama[i] && tt>-20 && tt<20) h_time_around_peak_ham->Fill(tt, charge);

    }
    
    // Debug 

    if (debug_plot_fit_result)
    {
        TCanvas* c1 = new TCanvas("c_rising", "c_rising", 800,600);
        h_time_rising_edge->Draw("hist");
        TCanvas* c2 = new TCanvas("c_falling", "c_falling", 800,600);
        h_time_falling_edge->Draw("hist");
        c1->SaveAs((TString)c1->GetTitle()+"_"+evtID+".png");
        c2->SaveAs((TString)c2->GetTitle()+"_"+evtID+".png");
        delete c1;
        delete c2;
    }

    m_constant_to_save = FitToGetTimeConstants(0);
    
    //------------calculate w1 and w2
    
    if (debug_plot_fit_result)
    {
        TCanvas* c1 = new TCanvas("c_around_peak", "c_around_peak", 800,600);
        h_time_around_peak_ham->Draw("hist");
        c1->SaveAs((TString)c1->GetTitle()+"_"+evtID+".png");
    }

    int ss = 0; // option for function GetW*(), which means only get integral area=0.1
    h_time_around_peak_ham->Scale(1./h_time_around_peak_ham->Integral());
    m_w1 = GetW1(h_time_around_peak_ham, ss);
    m_w2 = GetW2(h_time_around_peak_ham, ss);
    
    // -------Get Event Vertex-------------
    m_vertex_r3_of_event =  m_psdInput->getVertexR3();
    m_E_rec = m_psdInput->getErec();

    // Only when disable Predict mode is on, we will save the fitting outputs so that we can do the training in the next step
    if (!b_usePredict) 
    {
        m_userTree->Fill();
        evtID ++;
    }

    return true;
}

double PSD_TMVA::predict(){
    cout<< "-------Try to Predict----"<<endl;
    m_psdEvent.psdVal = (double)(reader_BDTG->EvaluateMVA("BDTG method"));
    cout<< "BDTG Output:\t"<< m_psdEvent.psdVal <<endl;
    cout<< "--------------------------"<<endl;
    
    // Set default tag for prediction output
    if (m_psdEvent.psdVal>=0) m_psdEvent.evtType = EVTTYPE::Electron;
    else m_psdEvent.evtType = EVTTYPE::Proton;

    m_psdTree->Fill();
    evtID ++;
    return m_psdEvent.psdVal;
}
