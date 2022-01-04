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
#include "SniperKernel/SniperDataPtr.h"
#include <boost/python/numpy.hpp>
#include <vector>

#include "TTree.h"
#include "TH1D.h"
#include "string.h"

using namespace std;

class PyDataStore;

class PSDTool_sklearn: public ToolBase, public IPSDTool{
public:
    explicit PSDTool_sklearn(const std::string &name);
    ~PSDTool_sklearn() override;

    bool initialize() override;
    bool finalize() override;
    virtual bool preProcess(JM::EvtNavigator *);
    double CalPSDVal() override;
    void SetBins();

private:
    TTree *m_userTree;
    TTree *m_psdTree;
    vector<double> m_bins;

    IPSDInputSvc* m_psdInput;

    std::string m_path_model;
    std::string m_output_file;
    std::string m_path_bins_file;

    // histogram for training
    TH1D* m_h_time_without_charge;
    TH1D* m_h_time_with_charge;

    // Varibles for Training
    int evtID;
    vector<double> v_h_time_without_charge;
    vector<double> v_h_time_with_charge;
    vector<double> vertex_xyz;
    double Erec;

    // Python DataStore
    PyDataStore* m_ds;
};
#endif //GENERALPROJECT_PSDTOOL_SKLEARN_H
