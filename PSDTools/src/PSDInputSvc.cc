#include "../PSDTools/PSDInputSvc.h"

#include "DataRegistritionSvc/DataRegistritionSvc.h"
#include "Event/ElecHeader.h"
#include "Identifier/Identifier.h"
#include "Identifier/CdID.h"
#include "SniperKernel/SvcFactory.h"
#include "SniperKernel/SniperLog.h"
#include "EvtNavigator/NavBuffer.h"
#include "BufferMemMgr/IDataMemMgr.h"
#include "SniperKernel/SniperPtr.h"
#include "RootWriter/RootWriter.h"
#include "DataRegistritionSvc/DataRegistritionSvc.h"
#include "Event/CalibHeader.h"

#include <map>
#include <iomanip>

using namespace std;

///// definition for Service
DECLARE_SERVICE(PSDInputSvc);

typedef pair<int, double> PAIR;
int cmp(const PAIR& x, const PAIR& y)  //
{
  return x.second > y.second;
}
double GetTimetag(const TH1F* h1tem)
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

PSDInputSvc::PSDInputSvc(const std::string& name) : SvcBase(name)
{
  b_weightOpt = false;
  hist_to_align = new TH1F("h_to_align", "", 2000, -200, 1800);
  hist_to_align_Ham = new TH1F("h_to_align_Ham", "", 2000, -200, 1800);
  hist_to_align_MCP = new TH1F("h_to_align_MCP", "", 2000, -200, 1800);
  declProp("PMT_Map", pmt_map);
}

PSDInputSvc::~PSDInputSvc()
= default;

bool PSDInputSvc::initialize()
{
  Initialize_PMT_Map(pmt_map);
  return true;
}

bool PSDInputSvc::finalize()
{
  return true;
}

bool PSDInputSvc::extractHitInfo(JM::EvtNavigator* nav, const std::string method_to_align)
{

  // read time and charge fromJM::CalibEvent *calibevent, JM::CDRecEvent *recevent CalibEvent
  if (!getEDMEvent(nav)) return false;
  if (!extractEvtInfo(nav)) return false;

  v_hitTime.clear();
  v_hitTime.reserve(10000);
  v_hitCharge.clear();
  v_hitCharge.reserve(10000);
  v_hitIsHama.clear();
  v_hitIsHama.reserve(10000);

  const std::list<JM::CalibPMTChannel*>& l_pmtcol = calibEvent->calibPMTCol();
  for (auto it : l_pmtcol) {
    unsigned int pmtid = it->pmtId();
    Identifier id = Identifier(pmtid);
    if (not CdID::is20inch(id) || CdID::module(id) >= m_entries_LPMT) {
      continue;
    }

    int hitno = it->size();
    const std::vector<double>& hittime = it->time();
    const std::vector<double>& charge = it->charge();
    for (int i = 0; i < hitno; i++) {
      // v_pmtID.push_back(CdID::module(id));
      v_hitTime.push_back(hittime.at(i) - calTOF(CdID::module(id)));
      v_hitCharge.push_back(charge.at(i));
      v_hitIsHama.push_back(v_PMTMap_isHama.at(CdID::module(id)));
    }
  }
  if (!alignTime(method_to_align)) return false;

  return true;
}

bool PSDInputSvc::extractHitsWaveform(JM::EvtNavigator* nav)
{
  v2d_waveforms.clear();
  v2d_waveforms.reserve(10000);

  ////////////////////// Load waveforms of hits /////////////////////////////////
  if (!getEDMEvent(nav)) return false;  // get elecEvent from navigator
  if (!extractEvtInfo(nav)) return false;

  const JM::ElecFeeCrate& efc = elecEvent->elecFeeCrate();
  auto* m_crate = const_cast<JM::ElecFeeCrate*>(&efc);

  map<int, JM::ElecFeeChannel> feeChannels = m_crate->channelData();

  map<int, JM::ElecFeeChannel>::iterator it;
  for (it = feeChannels.begin(); it != feeChannels.end(); ++it) {
    JM::ElecFeeChannel& channel = (it->second);
    if (channel.adc().empty()) {
      continue;
    }

    int pmtID = it->first;  // remeber to check the conversion from electronics id to pmt id
    if (pmtID >= m_entries_LPMT) continue;

    vector<unsigned int>& waveform = channel.adc();
    v2d_waveforms.push_back(waveform);
  }
  return true;
}

bool PSDInputSvc::alignTime(std::string algnmethod)
{
  //-------shift the time by t0------------
  if (!algnmethod.compare("noShift")) return true;
  double t0 = 0;
  double t0_Ham = 0;
  double t0_MCP = 0;
  if (!algnmethod.compare("alignPeak")) {
    hist_to_align->Reset();
    int isize = v_hitTime.size();
    for (int i = 0; i < isize; i++) {
      double dweight = 1;
      if (b_weightOpt) dweight = v_hitCharge.at(i);
      hist_to_align->Fill(v_hitTime.at(i), dweight);
    }
    int binno = hist_to_align->GetMaximumBin();
    t0 = hist_to_align->GetBinCenter(binno) - 100;  // the peak aligned to 100

    for (double & it : v_hitTime) {
      it -= t0;
    }
  } else if (!algnmethod.compare("alignMean")) {
    double sum = 0;
    double totcharge = 0;
    int isize = v_hitTime.size();
    for (int i = 0; i < isize; i++) {
      double dweight = 1;
      if (b_weightOpt) dweight = v_hitCharge.at(i);
      totcharge += dweight;
      sum += v_hitTime.at(i) * dweight;
    }
    sum /= totcharge;
    t0 = sum - 100;

    for (double & it : v_hitTime) {
      it -= t0;
    }
  } else if (!algnmethod.compare("alignPeak2")) {
    b_weightOpt = true;
    t0 = 0;
    hist_to_align->Reset();
    hist_to_align_Ham->Reset();
    hist_to_align_MCP->Reset();

    int isize = v_hitTime.size();
    for (int i = 0; i < isize; i++) {
      double dweight = 1;
      if (b_weightOpt) dweight = v_hitCharge.at(i);
      hist_to_align->Fill(v_hitTime.at(i), dweight);
      if (v_hitIsHama.at(i))
        hist_to_align_Ham->Fill(v_hitTime.at(i), dweight);
      else
        hist_to_align_MCP->Fill(v_hitTime.at(i), dweight);
    }
    t0 = GetTimetag(hist_to_align) - 1;
    t0_Ham = GetTimetag(hist_to_align_Ham) - 1;
    cout << "My puzzle-GetTimetag():\t" << t0_Ham << endl;
    cout << "My puzzle-GetMaximum():\t" << hist_to_align_Ham->GetBinCenter(hist_to_align_Ham->GetMaximumBin()) << endl;
    cout << endl;
    t0_MCP = GetTimetag(hist_to_align_MCP) - 1;

    for (int i = 0; i < v_hitTime.size(); i++) {
      if (v_hitIsHama[i])
        v_hitTime[i] = v_hitTime[i] - t0_Ham;
      else
        v_hitTime[i] = v_hitTime[i] - t0_MCP;
    }
    //        int binno=hist_to_align_Ham->GetMaximumBin();
    //        t0=hist_to_align_Ham->GetBinCenter(binno)-1;
  }

  return true;
}

double PSDInputSvc::calTOF( const int id)
{

  // calculate the time of flight
  //	double PMT_R = 19.434;
  //	double Ball_R = 19.18;
  //  TVector3 pmtposi=Ball_R/PMT_R*v_PMTPosi.at(id);

  //-----------------------Jie changes at 2021/11/25
  TVector3 pmtposi = v_PMTPosi.at(id);

  // std::cout<<id<<":("<<pmtposi.X()<<", "<<pmtposi.Y()<<", "<<pmtposi.Z()<<")"<<std::endl;

  double dx = (d_vtxX - pmtposi.X()) / 1000.;
  double dy = (d_vtxY - pmtposi.Y()) / 1000.;
  double dz = (d_vtxZ - pmtposi.Z()) / 1000.;
  double dist = TMath::Sqrt(dx * dx + dy * dy + dz * dz);
  double lightspeed = 299792458 / 1e9;
  double RB = sqrt(pow(pmtposi.X(), 2) + pow(pmtposi.Y(), 2) + pow(pmtposi.Z(), 2)) * 1e-3;
  double RR = sqrt(pow(d_vtxX, 2) + pow(d_vtxY, 2) + pow(d_vtxZ, 2)) * 1e-3;
  double costheta = RB * RB + dist * dist - RR * RR;
  costheta = costheta / (2 * RB * dist);
  double DB = RB * costheta - sqrt(17.7 * 17.7 - RB * RB * (1. - pow(costheta, 2)));
  double DLS = dist - DB;

  //--------------------------method A
  double m_neff = 1.578;
  double tof = dist * m_neff / lightspeed;

  //------------------------method B
  double nLSeff = 1.546;
  double nWReff = 1.373;
  double tof2 = DLS / (lightspeed / nLSeff) + DB / (lightspeed / nWReff);

  //std::cout << "TOF:" << tof << "\t" << tof2 << std::endl;

  // return tof;
  return tof2;
}
