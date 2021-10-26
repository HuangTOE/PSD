#include "../PSD/PSDInputSvc.h"

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
//#include "TString.h"
//
//#include "TH1.h"
//#include "TFile.h"
//#include "TTree.h"
//#include "TMath.h"
    
using namespace std;

///// definition for Service
DECLARE_SERVICE(PSDInputSvc);

PSDInputSvc::PSDInputSvc(const std::string& name)
: SvcBase(name)
{
    b_weightOpt = false;
    hist_to_align = new TH1F("h_to_align", "", 2000, -200, 1800);
    declProp("PMT_Map", pmt_map);
}

PSDInputSvc::~PSDInputSvc(){
}

bool PSDInputSvc::initialize()
{
    Initialize_PMT_Map(pmt_map);
    return true;
}

bool PSDInputSvc::finalize()
{
    return true;
}

bool PSDInputSvc::extractHitInfo(JM::EvtNavigator *nav, const std::string method_to_align){

    //read time and charge fromJM::CalibEvent *calibevent, JM::CDRecEvent *recevent CalibEvent
    if (!getEDMEvent(nav)) return false;
    if (!extractEvtInfo(nav)) return false;

    v_hitTime.clear();
    v_hitTime.reserve(10000);
    v_hitCharge.clear();
    v_hitCharge.reserve(10000);
    v_hitIsHama.clear();
    v_hitIsHama.reserve(10000);


    const std::list<JM::CalibPMTChannel*>& l_pmtcol = calibEvent->calibPMTCol();
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
            v_hitIsHama.push_back(v_PMTMap_isHama.at(CdID::module(id)));
        }
    }
    if(!alignTime(method_to_align)) return false;

    return true;
}

bool PSDInputSvc::extractHitsWaveform(JM::EvtNavigator *nav)
{
    v2d_waveforms.clear();
    v2d_waveforms.reserve(10000);

    ////////////////////// Load waveforms of hits /////////////////////////////////
    if (!getEDMEvent(nav)) return false; // get elecEvent from navigator
    if (!extractEvtInfo(nav)) return false;

    const JM::ElecFeeCrate &efc = elecEvent->elecFeeCrate();
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

bool PSDInputSvc::alignTime(std::string algnmethod){
    //-------shift the time by t0------------
    if (!algnmethod.compare("noShift")) return true;
    double t0=0;
    if (!algnmethod.compare("alignPeak")){
        hist_to_align->Reset();
        int isize=v_hitTime.size();
        for (int i=0;i<isize;i++){
            double dweight=1;
            if (b_weightOpt) dweight=v_hitCharge.at(i);
            hist_to_align->Fill(v_hitTime.at(i),dweight);
        }
        int binno=hist_to_align->GetMaximumBin();
        t0=hist_to_align->GetBinCenter(binno)-100;//the peak aligned to 100
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
    return true;
}


double PSDInputSvc::calTOF(int id){

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
