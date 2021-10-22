#ifndef ISNPSDTOOL_H
#define ISNPSDTOOL_H

#include "PSD/IPSDTool.h"

#include "TFile.h"
#include "TTree.h"

#include <string>

class ISNPSDTool: public IPSDTool{
    public:
        ISNPSDTool();
        virtual ~ISNPSDTool();

        virtual bool initialize();
        virtual bool finalize();

        virtual bool init() = 0;
        virtual bool fina() = 0;

    protected:
        //--------------get functions---------------
        std::string getTruthEvtType(int);

        //-------------------options------------------------
        std::string s_truthFileName;

    private:
        TFile *m_truthFile;
        TTree *m_truthTree;
        std::string *s_truthEvtType;
};
#endif
