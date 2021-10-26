#ifndef PSDALG_H
#define PSDALG_H
//#pragma once

#include "../PSD/IPSDTool.h"

#include "SniperKernel/AlgBase.h"
#include "EvtNavigator/NavBuffer.h"

#include "TTree.h"

class PSDAlg:public AlgBase{
    public:
        PSDAlg(const std::string &name);
        ~PSDAlg();

        bool initialize();
        bool execute();
        bool finalize();

    private:
        //----------------Options-------------
        bool b_usePredict;
        std::string s_psdMethod;
        std::string b_model;

        int i_ithEvt;
        JM::NavBuffer *m_buf;
        IPSDTool *m_psdTool;

        //============for output info: eg. PSD event type===========
        TTree *m_outTree;
        double d_psdVal;
        double d_recE;
        double d_recX, d_recY, d_recZ;
};
#endif
