#ifndef SNMLTOOL_H
#define SNMLTOOL_H

#include "ISNPSDTool.h"
#include "PSD/PSDInput.h"
#include "SniperKernel/ToolBase.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1.h"

class SNMLTool: public ToolBase, public ISNPSDTool{
    public:
        SNMLTool(const std::string &name);
        ~SNMLTool();

        virtual bool init();
        virtual bool fina();
        virtual bool preProcess(JM::CalibEvent*, JM::CDRecEvent*);
        virtual double predict();

    private:
        int i_ithEvt;
        static const int i_NEvtTypes=10;
        //const std::string s_evtType[i_NEvtTypes]={"IBD", "IBDp", "IBDd", "pES", "eES", "C12", "N12", "B12", "AfterPulse", "pileUp"};

        //-------------for input--------------
        PSDInput *m_psdInput;
        std::string s_weightFileName;
        TFile *m_weightFile;
        TTree *m_psdValTree;
        double d_predictVal;

        //-------------for output--------------
        //std::map<std::string, TTree*> m_userTree;//output of PSDTools Var
        TTree *m_userTree;//output of PSDTools Var
        TTree *m_psdTree;

        //---------------common---------------
        int i_nBins;
        double d_lowEdge;
        double d_highEdge;
        TH1D *m_hitTimeForEvt;
        double *d_psdVar;
        double d_recE;
        std::string s_truthEvtType;

        //-------------for output--------------
        bool initForPredict();

};
#endif
