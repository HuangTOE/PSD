//
// Created by luoxj@ihep.ac.cn on 2021/10/24.
//

#include "PSDTools/IPSDInputSvc.h"
#include "SniperKernel/SniperLog.h"
#include "SniperKernel/SniperDataPtr.h"
#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "Geometry/PMTParamSvc.h"
#include "EvtNavigator/EvtNavHelper.h"

using namespace std;

IPSDInputSvc::IPSDInputSvc() {
    v_hitTime.clear();
    v_hitCharge.clear();
    d_vtxX = 0;
    d_vtxY = 0;
    d_vtxZ = 0;
}

IPSDInputSvc::~IPSDInputSvc() = default;

bool IPSDInputSvc::Initialize(PMTParamSvc* pmt_svc) {
    m_pmt_svc = pmt_svc;
    return true;
}

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


bool IPSDInputSvc::getEDMEvent(JM::EvtNavigator *nav) {
    auto eh = JM::getHeaderObject<JM::CdWaveformHeader>(nav);
    // only use large pmts
    if (eh!=NULL)
    {
        if (!eh->hasEvent()) {
            LogError << "Cannot load elecsim event!!!!Please check input calib file" << endl;
            return false;
        }
        elecEvent = dynamic_cast<JM::CdWaveformEvt *>(eh->event());
    }
    else
        LogWarn << "Cannot find elecsim input, so we skip getting waveform!" << endl;

    //read time and charge fromJM::CalibEvent *calibevent, JM::CDRecEvent *recevent CalibEvent
    auto calibheader=JM::getHeaderObject<JM::CdLpmtCalibHeader>(nav);
    if (!calibheader->hasEvent()) {
        LogError << "Cannot load calib event!!!!Please check input calib file" << endl;
        return false;
    }
    calibEvent=calibheader->event();
    auto recheader=JM::getHeaderObject<JM::CdRecHeader>(nav);
    if (!recheader->hasEvent()) {
        LogError << "Cannot load rec event!!!!Please check input rec file" << endl;
        return false;
    }
    recEvent=recheader->event();
    return true;
}


vector<double> IPSDInputSvc::getEventXYZ() {
    std::vector<double> v_XYZ = {d_vtxX, d_vtxY, d_vtxZ};
    return v_XYZ;
}





