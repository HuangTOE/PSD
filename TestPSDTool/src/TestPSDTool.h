#ifndef TESTPSDTOOL_H
#define TESTPSDTOOL_H

#include "PSD/IPSDTool.h"
#include "PSD/PSDInput.h"
#include "SniperKernel/ToolBase.h"
#include "EvtNavigator/NavBuffer.h"

#include "TTree.h"

#include <string>

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

        PSDInput *m_psdInput;
};
#endif
