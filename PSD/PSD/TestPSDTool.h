#ifndef TESTPSDTOOL_H
#define TESTPSDTOOL_H
//#pragma once

#include "../PSD/IPSDTool.h"
#include "../PSD/IPSDInputSvc.h"
#include "SniperKernel/ToolBase.h"
#include "EvtNavigator/NavBuffer.h"
#include "SniperKernel/SniperPtr.h"

#include "TTree.h"

class TestPSDTool: public ToolBase, public IPSDTool{
    public:
        TestPSDTool(const std::string &name);
        ~TestPSDTool();

        virtual bool initialize();
        virtual bool finalize();
        virtual bool preProcess(JM::EvtNavigator *);
        virtual double CalPSDVal();

    private:
        TTree *m_userTree;
        double d_psdVar;
        TTree *m_psdTree;

        IPSDInputSvc* m_psdInput;
};
#endif
