//
// Created by luoxj@ihep.ac.cn on 2021/10/24.
/*
 * Author: Xin Huang, Xiaojie Luo
 * IPSDInputSvc is the base class to prepare necessary input for PSD work, PSDInputSvc is the default service.
 * If input provided by PSDInputSvc cannot satisfy your PSDTools, you can develop your own  PSDInputSvc
 */
//

#pragma once
//#ifndef IPSDINPUTSVC_H
//#define IPSDINPUTSVC_H

#include "EvtNavigator/NavBuffer.h"
#include "Event/CdWaveformHeader.h"
#include "Event/CdLpmtCalibHeader.h"
#include "Event/CdRecHeader.h"
#include <string>
#include "TString.h"
#include <vector>
#include "TVector3.h"
#include "Geometry/PMTParamSvc.h"

class IPSDInputSvc {
public:

    IPSDInputSvc();
    bool Initialize(PMTParamSvc*);
    virtual ~IPSDInputSvc();

    // Prepare Reader from EDM
    bool getEDMEvent(JM::EvtNavigator *);

    //////////////////////////////////////////////////////////////////////////////////
    // functions for extracting hits' time and charge information
    // virtual functions for extracting information from EDM and do some preprocessing
    // like subtracting time of fly

    // Manipulation about T/Q from waveform construction to extract emission time
    virtual bool extractHitInfo(JM::EvtNavigator * , std::string method_to_align);

    // Manipulation about waveforms from electronic simulation  to extract time profile
    virtual bool extractHitsWaveform(JM::EvtNavigator *);

    // Extract event information from EDM such as reconstruct vertex and energy
    virtual bool extractEvtInfo(JM::EvtNavigator *);
    //////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////
    // functions for getting information from the extracting functions above
    const std::vector<double>& getHitTime(){return v_hitTime;};
    const std::vector<double>& getHitCharge(){return v_hitCharge;};
    const std::vector<int   >& getHitIsHama(){return v_hitIsHama;}
    const std::vector<int   >& getHitPMTID(){return v_hitPMTID;}
    std::vector<std::vector<unsigned int >> getHitsWaveform(){return v2d_waveforms;};
    std::vector<double> getEventXYZ();
    double getVertexR3() const {return d_R3_event;}
    double getErec() const {return m_E_rec;}
    /////////////////////////////////////////////////////////////////////////////////

protected:
    // event information in current event
    double d_vtxX{};
    double d_vtxY{};
    double d_vtxZ{};
    double d_R3_event{}; // (x^2+y^2+z^2)^1.5, unit: m
    double m_E_rec{};    // reconstruct Energy

    // pmt svc for the whole detector geometry
    PMTParamSvc* m_pmt_svc;

    // hits information in one event ( first index is for the index of hit,
    // same index refer to the same hit in different variables below )
    std::vector<double> v_hitTime; // hit time of many hits in current event
    std::vector<double> v_hitCharge;//hit charge of many hits in current event
    std::vector<int> v_hitPMTID;
    std::vector<int> v_hitIsHama;
    std::vector<std::vector<unsigned int >> v2d_waveforms;

    // EDM in offline for current event to get related information
    JM::CdLpmtCalibEvt *calibEvent{};
    JM::CdRecEvt *recEvent{};
    JM::CdWaveformEvt * elecEvent{};


};


//#endif //IPSDINPUTSVC_H
