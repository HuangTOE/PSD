#ifndef PSDALG_H
#define PSDALG_H

/*
 * Author: Xin Huang, Xiaojie Luo
 * PSDAlg is like the "main" function in the whole PSDTools, which pass EDM navigator to PSDInputSvc then invoke PSDTool.
 */

#include "../PSDTools/IPSDTool.h"

#include "SniperKernel/AlgBase.h"
#include "EvtNavigator/NavBuffer.h"

#include "TTree.h"

class PSDAlg:public AlgBase{
    public:
        explicit PSDAlg(const std::string &name);
        ~PSDAlg() override;

        bool initialize() override;
        bool execute() override;
        bool finalize() override;

    private:
        //----------------Options-------------
        bool b_usePredict;
        std::string s_psdMethod;
        std::string b_model;

        int i_ithEvt;
        JM::NavBuffer *m_buf{};
        IPSDTool *m_psdTool;

        //============for output info: eg. PSDTools event type===========
        TTree *m_outTree{};
        double d_psdVal;
        double d_recE;
        double d_recX, d_recY, d_recZ;
};
#endif
