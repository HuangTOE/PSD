//
// Created by luoxj@ihep.ac.cn on 2021/10/24.
//

#include "../PSD/IPSDInputSvc.h"
#include "SniperKernel/SniperLog.h"
#include <vector>
#include "Event/ElecHeader.h"
#include "Event/CalibHeader.h"
#include "Event/RecHeader.h"
#include "TFile.h"
#include "TTree.h"


using namespace std;

IPSDInputSvc::IPSDInputSvc() {
    v_PMTPosi.clear();
    v_PMTMap_isHama.clear();
    v_hitTime.clear();
    v_hitCharge.clear();
    d_vtxX = 0;
    d_vtxY = 0;
    d_vtxZ = 0;
}

IPSDInputSvc::~IPSDInputSvc() = default;

bool IPSDInputSvc::extractHitInfo(JM::EvtNavigator * nav, const std::string method_to_align) {
    if (!getEDMEvent(nav)) return false;
    cout << method_to_align << endl;
    return false;
}


bool IPSDInputSvc::extractHitsWaveform(JM::EvtNavigator * nav) {
    if (!getEDMEvent(nav)) return false;
    return false;
}


bool IPSDInputSvc::extractEvtInfo(JM::EvtNavigator * nav) {
    if (!getEDMEvent(nav)) return false;
    //read vertex from RecEvent
    d_vtxX = recEvent->x();
    d_vtxY = recEvent->y();
    d_vtxZ = recEvent->z();
    d_R3_event =pow( pow(d_vtxX/1000,2)+pow(d_vtxY/1000,2)+pow(d_vtxZ/1000,2), 1.5 );

    m_E_rec= recEvent->energy();
    return true;
}


bool IPSDInputSvc::getEDMEvent(JM::EvtNavigator * nav) {
    JM::ElecHeader *eh = dynamic_cast<JM::ElecHeader *>(nav->getHeader("/Event/Elec"));
    // only use large pmts
    if (!eh->hasEvent()) {
        LogError << "Cannot load elecsim event!!!!Please check input calib file" << endl;
        return false;
    }
    elecEvent = dynamic_cast<JM::ElecEvent *>(eh->event());

    //read time and charge fromJM::CalibEvent *calibevent, JM::CDRecEvent *recevent CalibEvent
    JM::CalibHeader *calibheader=dynamic_cast<JM::CalibHeader*>(nav->getHeader("/Event/Calib"));
    if (!calibheader->hasEvent()) {
        LogError << "Cannot load calib event!!!!Please check input calib file" << endl;
        return false;
    }
    calibEvent=calibheader->event();
    JM::RecHeader *recheader=dynamic_cast<JM::RecHeader*>(nav->getHeader("/Event/Rec"));
    if (!recheader->hasCDEvent()) {
        LogError << "Cannot load rec event!!!!Please check input rec file" << endl;
        return false;
    }
    recEvent=recheader->cdEvent();
    return true;
}

bool IPSDInputSvc::Initialize_PMT_Map( TString name_file_pmt_map ) {
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

std::vector<double> IPSDInputSvc::getEventXYZ() {
    std::vector<double> v_XYZ = {d_vtxX, d_vtxY, d_vtxZ};
    return v_XYZ;
}





