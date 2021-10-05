#ifndef TESTPSDTOOL_H
#define TESTPSDTOOL_H

#include "IPSDTool.h"
#include "SniperKernel/ToolBase.h"

#include "TTree.h"

#include <string>

class TestPSDTool: public ToolBase, public IPSDTool{
    public:
        TestPSDTool(const std::string &name);
        ~TestPSDTool();

        virtual bool initialize();
        virtual bool finalize();
        virtual bool preProcess(JM::CalibEvent*, JM::CDRecEvent*);
        virtual double predict();

    private:
        TTree *m_userTree;
        double d_psdVar;
        TTree *m_psdTree;
};
#endif
