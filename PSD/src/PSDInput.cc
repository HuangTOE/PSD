#include "PSD/PSDInput.h"

#include "DataRegistritionSvc/DataRegistritionSvc.h"
#include "Event/ElecHeader.h"
#include "Identifier/Identifier.h"
#include "Identifier/CdID.h"
#include "SniperKernel/SniperLog.h"
#include "EvtNavigator/NavBuffer.h"
#include "BufferMemMgr/IDataMemMgr.h"
#include "SniperKernel/SniperPtr.h"
#include "RootWriter/RootWriter.h"
#include "DataRegistritionSvc/DataRegistritionSvc.h"
#include "Event/ElecHeader.h"
#include "Event/CalibHeader.h"


#include <map>
#include <iomanip>
#include "TString.h"

#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
    
using namespace std;
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

PSDInput::PSDInput(){
    b_weightOpt = false;
    v_PMTPosi.clear();
    v_PMTMap_isHama.clear();
    v_hitTime.clear();
    v_hitCharge.clear();
    Initialize_PMT_Map();
    d_vtxX = 0;
    d_vtxY = 0;
    d_vtxZ = 0;
    hist_to_align = new TH1F("h_to_align", "", 2000, -200, 1800);
}

PSDInput::~PSDInput(){
}

bool PSDInput::Initialize_PMT_Map( TString name_file_pmt_map)
{
       //read the position info from the root file
       v_PMTPosi.reserve(18000);
       v_PMTMap_isHama.reserve(18000);
       TFile *thisfile = TFile::Open(name_file_pmt_map);
       TTree *pmtdata=dynamic_cast<TTree*>(thisfile->Get("PmtData_Lpmt"));
       double pmtPosX=0;
       double pmtPosY=0;
       double pmtPosZ=0;
       int is_Hama = 0;
       pmtdata->SetBranchAddress("pmtPosX", &pmtPosX);
       pmtdata->SetBranchAddress("pmtPosY", &pmtPosY);
       pmtdata->SetBranchAddress("pmtPosZ", &pmtPosZ);
       pmtdata->SetBranchAddress("MCP_Hama",&is_Hama);
       m_entries_LPMT=pmtdata->GetEntries();
       for (int ith=0;ith<m_entries_LPMT;ith++){
           pmtdata->GetEntry(ith);
           TVector3 tmpv(pmtPosX, pmtPosY, pmtPosZ);
           v_PMTPosi.push_back(tmpv);
           v_PMTMap_isHama.push_back(is_Hama);
       }

       return true;

}
bool PSDInput::extractHitInfo(JM::EvtNavigator *nav){

    //read time and charge fromJM::CalibEvent *calibevent, JM::CDRecEvent *recevent CalibEvent
    JM::CalibHeader *calibheader=dynamic_cast<JM::CalibHeader*>(nav->getHeader("/Event/Calib"));
    JM::CalibEvent *calibevent=calibheader->event();
    JM::RecHeader *recheader=dynamic_cast<JM::RecHeader*>(nav->getHeader("/Event/Rec"));
    JM::CDRecEvent *recevent=recheader->cdEvent();

    v_hitTime.clear();
    v_hitTime.reserve(10000);
    v_hitCharge.clear();
    v_hitCharge.reserve(10000);
    v_isHama.clear();
    v_isHama.reserve(10000);
    d_vtxX = 0;d_vtxY = 0;d_vtxZ = 0;

    //read vertex from RecEvent
    d_vtxX = recevent->x();
    d_vtxY = recevent->y();
    d_vtxZ = recevent->z();
    m_E_rec= recevent->energy();

    d_R3_event =pow( pow(d_vtxX/1000,2)+pow(d_vtxY/1000,2)+pow(d_vtxZ/1000,2), 1.5 );

    const std::list<JM::CalibPMTChannel*>& l_pmtcol = calibevent->calibPMTCol();
    for (std::list<JM::CalibPMTChannel*>::const_iterator it = l_pmtcol.begin();it != l_pmtcol.end();++it){
        unsigned int pmtid = (*it)->pmtId();
        Identifier id = Identifier(pmtid);
        if (not CdID::is20inch(id) || CdID::module(id)>=m_entries_LPMT) {
            continue;
        }

        int hitno = (*it)->size();
        const std::vector<double>& hittime = (*it)->time();
        const std::vector<double>& charge = (*it)->charge();
        for (int i = 0;i<hitno;i++){
            //v_pmtID.push_back(CdID::module(id));
            v_hitTime.push_back(hittime.at(i)-calTOF(CdID::module(id)));
            v_hitCharge.push_back(charge.at(i));
            v_isHama.push_back(v_PMTMap_isHama.at(CdID::module(id)));
            
        }
    }
    return true;
}

bool PSDInput::extractHitsWaveform(JM::EvtNavigator *nav)
{
    v2d_waveforms.clear();
    v2d_waveforms.reserve(10000);

    ////////////////////// Load waveforms of hits /////////////////////////////////
    JM::ElecHeader *eh = dynamic_cast<JM::ElecHeader *>(nav->getHeader("/Event/Elec"));

    // only use large pmts
    if (!eh->hasEvent()) return true;
    JM::ElecEvent *ee = dynamic_cast<JM::ElecEvent *>(eh->event());
    const JM::ElecFeeCrate &efc = ee->elecFeeCrate();
    JM::ElecFeeCrate * m_crate = const_cast<JM::ElecFeeCrate *>(&efc);
    
    map<int, JM::ElecFeeChannel> feeChannels = m_crate->channelData();
    
    map<int, JM::ElecFeeChannel>::iterator it;
    for (it = feeChannels.begin(); it != feeChannels.end(); ++it) 
    {
      JM::ElecFeeChannel &channel = (it->second);
      if (channel.adc().size() == 0) {
        continue;
      }

      int pmtID = it->first;  // remeber to check the conversion from electronics id to pmt id
      if (pmtID >= m_entries_LPMT) continue;
     
      vector<unsigned int> &waveform = channel.adc();
      v2d_waveforms.push_back(waveform);
    }
    return true;

}

void PSDInput::alignTime(std::string algnmethod){
    //-------shift the time by t0------------
    if (!algnmethod.compare("noShift")) return;
    double t0=0;
    if (!algnmethod.compare("alignPeak")){
        TH1D *hist_to_align=new TH1D("hist_to_align","title",550,-100,1000);
        int isize=v_hitTime.size();
        for (int i=0;i<isize;i++){
            double dweight=1;
            if (b_weightOpt) dweight=v_hitCharge.at(i);
            hist_to_align->Fill(v_hitTime.at(i),dweight);
        }
        int binno=hist_to_align->GetMaximumBin();
        t0=hist_to_align->GetBinCenter(binno)-100;//the peak aligned to 100
        delete hist_to_align;
    }
    else if (!algnmethod.compare("alignMean")){
        double sum=0;
        double totcharge=0;
        int isize=v_hitTime.size();
        for (int i=0;i<isize;i++){
            double dweight=1;
            if (b_weightOpt) dweight=v_hitCharge.at(i);
            totcharge+=dweight;
            sum+=v_hitTime.at(i)*dweight;
        }
        sum/=totcharge;
        t0=sum-100;
    }
    else if (!algnmethod.compare("alignPeak2"))
    {
        b_weightOpt = true;
        t0 = 0;
        hist_to_align->Reset();
        int isize=v_hitTime.size();
        for (int i=0;i<isize;i++)
        {
            double dweight=1;
            if (b_weightOpt) dweight=v_hitCharge.at(i);
            hist_to_align->Fill(v_hitTime.at(i),dweight);
        }
        //double t0 = GetTimetag(hist_to_align)-1  ;
        int binno=hist_to_align->GetMaximumBin();
        t0=hist_to_align->GetBinCenter(binno)-1;//the peak aligned to 100
        
    }

    for (std::vector<double>::iterator it=v_hitTime.begin();it!=v_hitTime.end();++it){
        *it-=t0;
    }
}


double PSDInput::calTOF(int id){

    //calculate the time of flight
	double PMT_R = 19.434;
	double Ball_R = 19.18;
    TVector3 pmtposi=Ball_R/PMT_R*v_PMTPosi.at(id);
    //std::cout<<id<<":("<<pmtposi.X()<<", "<<pmtposi.Y()<<", "<<pmtposi.Z()<<")"<<std::endl;
    
    double dx=(d_vtxX-pmtposi.X())/1000.;
    double dy=(d_vtxY-pmtposi.Y())/1000.;
    double dz=(d_vtxZ-pmtposi.Z())/1000.;
	double dist = TMath::Sqrt(dx*dx+dy*dy+dz*dz);
    double m_neff=1.578;
	double lightspeed = 299792458/1e9;
    double tof=dist*m_neff/lightspeed;
    //std::cout<<"TOF:"<<tof<<std::endl;
	return tof;
}
