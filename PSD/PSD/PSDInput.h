#ifndef PSDINPUT_H
#define PSDINPUT_H

#include "TH1F.h"
#include <vector>
#include <string>
#include "TString.h"

#include "TVector3.h"

#include "Event/ElecHeader.h"
#include "EvtNavigator/NavBuffer.h"
#include "Event/CalibHeader.h"
#include "Event/RecHeader.h"

class PSDInput{
    public:
        PSDInput();
        ~PSDInput();

        bool extractHitInfo(JM::EvtNavigator *);
        bool extractHitsWaveform(JM::EvtNavigator *);
        void alignTime(std::string algnmethod = "");

        //get functions
        std::vector<double>& getHitTime(){return v_hitTime;}
        std::vector<double>& getHitCharge(){return v_hitCharge;}
        std::vector<int   >& getHitIsHama(){return v_isHama;}
        std::vector<std::vector<unsigned int >> getHitsWaveform() {return v2d_waveforms;}
        double getVertexR3() {return d_R3_event;}
        double getErec() {return m_E_rec;}

    private:
        //-----------options---------
        bool b_weightOpt;

        //-----------private functions---------
        double calTOF(int);
        bool Initialize_PMT_Map( TString name_file_pmt_map="/cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc830/Pre-Release/J21v1r0-Pre2/data/Simulation/ElecSim/PmtData_Lpmt.root" );

        //-----------private members---------
        std::vector<TVector3> v_PMTPosi;
        std::vector<int> v_PMTMap_isHama;
        std::vector<double> v_hitTime;
        std::vector<double> v_hitCharge;
        std::vector<int> v_isHama;
        std::vector<std::vector<unsigned int >> v2d_waveforms;
        double d_vtxX;double d_vtxY;double d_vtxZ; // unit: mm
        double d_R3_event; // m
        double m_E_rec;
        int m_entries_LPMT;
        TH1F* hist_to_align;
};
#endif
