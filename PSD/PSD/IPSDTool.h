//#ifndef IPSDTOOL_H
//#define IPSDTOOL_H
#pragma once

#include "EvtNavigator/EvtNavigator.h"
#include "Event/CalibEvent.h"
#include "Event/CDRecEvent.h"

#include <string>
#include <vector>

class IPSDTool{
    public:
        IPSDTool();
        virtual ~IPSDTool();

        enum EVTTYPE{
            Unknow, Electron, Proton
        };
        bool isElectron(){return (m_psdEvent.evtType == EVTTYPE::Electron);}
        bool isProton(){return (m_psdEvent.evtType == EVTTYPE::Proton);}

        void enablePredict(){b_usePredict = true;}
        void disablePredict(){b_usePredict = false;}
//        void setModelToPredict(std::string model){ b_model=model; }

        struct PSDEVENT{
            double psdVal;
            EVTTYPE evtType;
            PSDEVENT(){
                psdVal = -1;
                evtType = EVTTYPE::Unknow;
            }
        };

        virtual bool initialize() = 0;
        virtual bool finalize() = 0;
        virtual bool preProcess(JM::EvtNavigator *) = 0;
        virtual double CalPSDVal() = 0;

    protected:
        bool b_usePredict;//whether to give prediction or not, default is true
//        std::string b_model;
        PSDEVENT m_psdEvent;
};
//#endif
