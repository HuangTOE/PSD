#ifndef TESTPSDTOOL_H
#define TESTPSDTOOL_H
//#pragma once
// Created by luoxj@ihep.ac.cn on 2021/10/24.
/*
 * Author: Jie Cheng, Xiaojie Luo
 * PSDTool_TMVA is to use TMVA to do the PSD work by using parameters fitting from emission time profile
 */
#include "IPSDTool.h"
#include "IPSDInputSvc.h"
#include "SniperKernel/ToolBase.h"
#include "EvtNavigator/NavBuffer.h"

#include "TH1F.h"
#include "TMVA/Reader.h"
#include "TTree.h"
#include "vector"

struct fitRes {
  float tau1;
  float tau2;
  float tau1ratio;
  float ndark;
  float nn0;
  float chi2;
  float ndf;
  double tau1err;
  double tau2err;
  double tau1ratioerr;
};


class PSD_TMVA: public ToolBase, public IPSDTool{
    public:
        explicit PSD_TMVA(const std::string &name);
        ~PSD_TMVA() override;

        bool initialize() override;
        bool finalize() override;
        bool preProcess(JM::EvtNavigator *) override;
        double CalPSDVal() override;

    private:
        struct fitRes FitToGetTimeConstants( int fittag, double xhigh);

        TTree *m_userTree;
        TTree *m_psdTree;

        bool whether_with_charge;

        IPSDInputSvc *m_psdInput;

        TH1F* h_time;
        TH1F* h_time_rising_edge;
        TTree* tree_time_falling_edge;
        TH1F* h_time_falling_edge;
        
        // only Hamamtsu PMTs 
        TH1F* h_time_around_peak_ham ;
        TH1F* h_time_ham ; //------added by Jie in 2021/11/26
        std::vector<float> v_h_time_w;
        
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
        // For Two models
//        std::string model_FV1;
//        std::string model_FV2;
        std::string model;
        float R_divide;
        float PSD_divide;
        TMVA::Reader *reader_BDTG;

        float Equench;
        float nPE_tag;
        float v_tag;
        float id_tag;
        int isoz;
        int ison;

        float m_peak_to_tail_ratio;
        float m_tail_to_tail_ratio;

        float m_peak_to_total_ratio_ham; //----------added by Jie in 2021/11/26
        float m_tail_to_total_ratio_ham; //----------added by Jie in 2021/11/26

        
};
#endif
