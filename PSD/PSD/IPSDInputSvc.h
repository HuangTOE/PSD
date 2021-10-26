//
// Created by luoxj@ihep.ac.cn on 2021/10/24.
//

#pragma once
//#ifndef IPSDINPUTSVC_H
//#define IPSDINPUTSVC_H

#include "EvtNavigator/NavBuffer.h"
#include "Event/ElecHeader.h"
#include "Event/CalibHeader.h"
#include "Event/RecHeader.h"
#include <string>
#include "TString.h"
#include <vector>
#include "TVector3.h"

class IPSDInputSvc {
public:

    IPSDInputSvc();
    virtual ~IPSDInputSvc();

    // Prepare Reader from EDM
    bool getEDMEvent(JM::EvtNavigator *);

    // functions for extracting hits' time and charge information
    // virtual functions
    virtual bool extractHitInfo(JM::EvtNavigator * , const std::string method_to_align);
    virtual bool extractHitsWaveform(JM::EvtNavigator *);
    virtual bool extractEvtInfo(JM::EvtNavigator *);
    virtual bool Initialize_PMT_Map( TString name_file_pmt_map );

    // functions for getting information from the extracting functions above
    std::vector<double>& getHitTime(){return v_hitTime;};
    std::vector<double>& getHitCharge(){return v_hitCharge;};
    std::vector<int   >& getHitIsHama(){return v_hitIsHama;}
    std::vector<std::vector<unsigned int >> getHitsWaveform(){return v2d_waveforms;};
    std::vector<double> getEventXYZ();
    double getVertexR3() const {return d_R3_event;}
    double getErec() const {return m_E_rec;}

protected:
    // event information in current event
    double d_vtxX{};
    double d_vtxY{};
    double d_vtxZ{};
    double d_R3_event{}; // (x^2+y^2+z^2)^1.5, unit: m
    double m_E_rec{};    // reconstruct Energy

    // pmt map for the whole detector ( only large PMTs ), length is 17162 in offline J21v1
    Long64_t m_entries_LPMT{};
    std::vector<TVector3> v_PMTPosi;
    std::vector<int> v_PMTMap_isHama;

    // hits information in one event ( first index is for the index of hit,
    // same index refer to the same hit in different variables below )
    std::vector<double> v_hitTime; // hit time of many hits in current event
    std::vector<double> v_hitCharge;//hit charge of many hits in current event
    std::vector<int> v_hitIsHama;
    std::vector<std::vector<unsigned int >> v2d_waveforms;

    // EDM in offline for current event to get related information
    JM::CalibEvent *calibEvent;
    JM::CDRecEvent *recEvent;
    JM::ElecEvent * elecEvent;


};


//#endif //IPSDINPUTSVC_H
