#ifndef TESTPSDTOOL_H
#define TESTPSDTOOL_H
//#pragma once

#include "../PSD/IPSDTool.h"
#include "../PSD/IPSDInputSvc.h"
#include "SniperKernel/ToolBase.h"
#include "EvtNavigator/NavBuffer.h"

#include "TH1F.h"
#include "TMVA/Reader.h"
#include "TTree.h"

struct fitRes {
  float tau1;
  float tau2;
  float tau1ratio;
  float ndark;
  float nn0;
  float chi2;
  float ndf;
};


class PSD_TMVA: public ToolBase, public IPSDTool{
    public:
        PSD_TMVA(const std::string &name);
        ~PSD_TMVA();

        virtual bool initialize();
        virtual bool finalize();
        virtual bool preProcess(JM::EvtNavigator *);
        virtual double CalPSDVal();

    private:
        struct fitRes FitToGetTimeConstants( int fittag);

        TTree *m_userTree;
        TTree *m_psdTree;

        bool whether_with_charge;

        IPSDInputSvc *m_psdInput;

        TH1F* h_time_rising_edge;
        TTree* tree_time_falling_edge;
        TH1F* h_time_falling_edge;
        
        // only Hamamtsu PMTs 
        TH1F* h_time_around_peak_ham ;
        
        double tt;
        double charge;
        
        int evtID;

        bool debug_plot_fit_result;
        
        struct fitRes m_constant_to_save;
        
        float m_w1;
        float m_w2;
        float m_vertex_r3_of_event;
        float m_E_rec;
        
        // Prediction Part
        std::string model_FV1;
        std::string model_FV2;
        float R_divide;
        float PSD_divide;
        TMVA::Reader *reader_BDTG;

        float Equench;
        float nPE_tag;
        float v_tag;
        float id_tag;
        int isoz;
        int ison;


        
};
#endif
