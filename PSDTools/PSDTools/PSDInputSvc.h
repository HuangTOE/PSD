#pragma once

/*
 * Author: Xin Huang, Xiaojie Luo
 * PSDInputSvc is to do preprocess for data so that can be process by PSD Tools
 */
#include "IPSDInputSvc.h"

#include "TH1F.h"
#include <vector>
#include <string>
#include "TString.h"

#include "TVector3.h"

#include "SniperKernel/SvcBase.h"
#include "Event/ElecHeader.h"
#include "EvtNavigator/NavBuffer.h"
#include "Event/CalibHeader.h"
#include "Event/RecHeader.h"

class PSDInputSvc : public SvcBase, public IPSDInputSvc
{
    public:
        explicit PSDInputSvc(const std::string &name);
        ~PSDInputSvc() override;

        bool initialize() override;
        bool finalize() override;

        // override virtual function in IPSDInputSvc
        bool extractHitInfo(JM::EvtNavigator *, std::string method_to_align);
        bool extractHitsWaveform(JM::EvtNavigator *);

        // functions in this class
        bool alignTime(std::string algnmethod = "");


    private:
        //-----------options---------
        bool b_weightOpt; // whether using time profile with weighting charge

        //-----------private functions---------
        double calTOF(int);

        //-----------private members---------
        TH1F* hist_to_align;
        TH1F* hist_to_align_Ham;
        TH1F* hist_to_align_MCP;
};
//#endif
