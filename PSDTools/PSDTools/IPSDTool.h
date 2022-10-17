//
// Created by luoxj@ihep.ac.cn on 2021/10/24.
/*
 * Author: Xin Huang, Xiaojie Luo
 * IPSDTool is the base class for PSDTools, PSD methods are supposed to inherited from this class.
 * If you want to develop your own PSD tools, there are several functions you need to implement:
 * 1. initialize()
 * 2. finalize()
 * 3. preProcess() -> Process hits or waveforms from PSDInputSvc to get necessary variables for PSD such as histogram about emission time
 *                     For machine learning,here you can prepare variables for training
 * 4. CalPSDVal() -> Use information from preProcess() to get the final PSD output to separate particles
 *                  For machine learning, here you can use model to process events so to get PSD output
 */
#pragma once


#include "EvtNavigator/EvtNavigator.h"

#include <string>
#include <vector>

class IPSDTool{
    public:
        IPSDTool();
        virtual ~IPSDTool();

        enum EVTTYPE{
            Unknow, Electron, Proton
        };
        bool isElectron() const{return (m_psdEvent.evtType == EVTTYPE::Electron);}
        bool isProton() const{return (m_psdEvent.evtType == EVTTYPE::Proton);}

        void enablePredict(){b_usePredict = true;}
        void disablePredict(){b_usePredict = false;}

        struct PSDEVENT{
            double psdVal;
            EVTTYPE evtType;
            PSDEVENT(){
                psdVal = -1;
                evtType = EVTTYPE::Unknow;
            }
        };

        // Necessary for inherited PSD tools
        virtual bool initialize() = 0;
        virtual bool finalize() = 0;
        virtual bool preProcess(JM::EvtNavigator *) = 0;
        virtual double CalPSDVal() = 0;

    protected:
        std::string method_align;
        bool b_usePredict;//whether to give prediction or not, default is true
//        std::string b_model;
        PSDEVENT m_psdEvent;
};
//#endif
