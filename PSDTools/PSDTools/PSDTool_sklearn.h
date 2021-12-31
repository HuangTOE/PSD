//
// Created by luoxj@ihep.ac.cn on 2021/12/19.
//

#ifndef GENERALPROJECT_PSDTOOL_SKLEARN_H
#define GENERALPROJECT_PSDTOOL_SKLEARN_H


/*
 * Author:  Xiaojie Luo
 * Using Sklearn to do the PSD work, here we invoke code from python
 */

#include "IPSDTool.h"
#include "IPSDInputSvc.h"
#include "SniperKernel/ToolBase.h"
#include "EvtNavigator/NavBuffer.h"
#include "SniperKernel/SniperPtr.h"
#include <boost/python/numpy.hpp>


#include "TTree.h"

class PSDTool_sklearn: public ToolBase, public IPSDTool{
public:
    explicit PSDTool_sklearn(const std::string &name);
    ~PSDTool_sklearn() override;

    bool initialize() override;
    bool finalize() override;
    virtual bool preProcess(JM::EvtNavigator *);
    double CalPSDVal() override;

private:
    TTree *m_userTree;
    double d_psdVar;
    TTree *m_psdTree;

    IPSDInputSvc* m_psdInput;
};
#endif //GENERALPROJECT_PSDTOOL_SKLEARN_H
