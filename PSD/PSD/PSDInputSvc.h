//#ifndef PSDINPUTSvc_H
//#define PSDINPUTSvc_H
#pragma once

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
        PSDInputSvc(const std::string &name);
        ~PSDInputSvc();

        bool initialize();
        bool finalize();

        // override virtual function in IPSDInputSvc
        bool extractHitInfo(JM::EvtNavigator *, const std::string method_to_align);
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
        std::string pmt_map;
};
//#endif
