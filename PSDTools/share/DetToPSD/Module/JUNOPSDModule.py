# -*- coding:utf-8 -*-
# @Time: 2021/11/25 20:57
# @Author: Luo Xiaojie, Huang Xin
# @Email: luoxj@ihep.ac.cn, huangxin.ihep.ac.cn
# @File: JUNOPSDModule.py

"""
version 0.0 ( by Huang Xin ) : Initialize, Add TestPSDTool for PSDTools example
version 1.0 ( by Luo Xiaojie ): Add TMVA_Tool in DSNB Study
"""

import sys
import os
import Sniper
import logging
from .JUNOModule import JUNOModule
from .JUNOUtils import mh


##############################################################################
# JUNOPSDModule
##############################################################################

class JUNOPSDModule(JUNOModule):
    """

    """

    DATA_LOG_MAP = {"Test":0, "Debug":2, "Info":3, "Warn":4, "Error":5, "Fatal":6}

    def __init__(self):
        pass

    def register_options(self, parser):
        self.register_options_common(parser)
        self.register_options_TMVA(parser)
        self.register_options_Sklearn(parser)

    def register_options_common(self, parser):
        # ---------------input and output-----------------
        parser.add_argument("--inputSvc", default="PSDInputSvc", help="Which PSDTools input service will be use, default for PSDInputSvc.*")

        # --------------------For PSDTools--------------------------
        parser.add_argument("--method-PSD", default="TestPSDTool",
                            choices=["TestPSDTool", "PSD_TMVA","PSDTool_sklearn"],
                            help="The PSDTools method")
        parser.add_argument("--Predict", dest="usePredict",
                            action="store_true")
        parser.add_argument("--PrepareForTraining",
                            dest="usePredict", action="store_false")
        parser.add_argument("--Model",
                            default="model.pkl", help="ML model to do the prediction, default is for Sklearn method")
        parser.add_argument("--PSD-cut",type=float, default=0.5,help="Set PSDTools boundary for bkg and sig,so that tag the event")

        parser.set_defaults(usePredict = True)

    def register_options_TMVA(self, parser):
        # -------------------For TMVA method-----------------------------
        # parser.add_argument("--Model",
        # default="Induction_BDTG.weights.xml",
        # help="ML model to do the prediction, "
        #      "default is for TMVA method")
        # for two models
        # parser.add_argument("--model_FV1", default="Induction__BDTG.weights_FV1.xml", help="ML model to do the prediction, default is for TMVA method")
        # parser.add_argument("--model_FV2", default="Induction__BDTG.weights_FV2.xml", help="ML model to do the prediction, default is for TMVA method")
        # parser.add_argument("--R_divide", type=float, default=16, help="radius boundary to divide  and FV2")
        pass


    def register_options_Sklearn(self, parser):
        # ----------------- For Sklearn method -------------------------
        parser.add_argument("--Path_Bins",
                            default="None",
                            help="This file is the bins strategy for sklearn")
        parser.add_argument("--NotAppendErec", dest="appendE",
                            action="store_false")
        parser.set_defaults(appendE = True)

    def init(self, toptask, args):
        self.init_common(toptask, args)
        self.init_PSD(toptask, args)

        if args.method_PSD == "PSD_TMVA":
            self.init_TMVA_model(toptask, args)
        elif args.method_PSD == "PSDTool_sklearn":
            self.init_Sklearn_model(toptask,args )

    def init_common(self, topTask, args):
        topTask.setLogLevel(self.DATA_LOG_MAP[args.loglevel])
        topTask.setEvtMax(args.evtmax)

    def init_PSD(self, topTask, args):
        #=======================PSDTools related======================
        import PSDTools
        self.psdalg = topTask.createAlg("PSDAlg")
        self.psdsvc = topTask.createSvc(args.inputSvc)
        self.psdtool = self.psdalg.createTool(args.method_PSD)
        self.psdalg.property("Method").set(args.method_PSD)
        self.psdalg.property("UsePredict").set(args.usePredict)


    def init_TMVA_model(self, topTask, args):
        # For two models
        # self.psdtool.property("Model_FV1").set(args.model_FV1)
        # self.psdtool.property("Model_FV2").set(args.model_FV2)
        # self.psdtool.property("R_divide").set(args.R_divide)

        self.psdtool.property("Model").set(args.Model)
        self.psdtool.property("PSD_divide").set(args.PSD_cut)

    def init_Sklearn_model(self, toptask, args):
        # Create a python store
        import SniperPython
        toptask.createSvc("PyDataStoreSvc/DataStore")

        # Create one algorithm in C++ to convert the EDM data to numpy
        # Create another algorithm in Python to process data
        import PSDTools
        import SniperPython
        import PSDTools.PSDSklearn

        self.psdtool.property("Model").set(args.Model)
        self.psdtool.property("Path_Bins").set(args.Path_Bins)
        self.psdtool.property("PSD_divide").set(args.PSD_cut)
        self.psdtool.property("AppendErec").set(args.appendE)

        if args.usePredict:
            self.alg_sklearn = PSDTools.PSDSklearn.PSDSklearn("PSDSklearn")
            iotask = toptask.createTask("Task/TaskSklearn")
            iotask.createSvc("PyDataStoreSvc/DataStore")
            iotask.addAlg(self.alg_sklearn)

    def add_output_vec(self, output_vec, args):
        #=======================Output using framwork======================
        ##########################################################
        ###For the output of data model, PSDTools data model is not implemented yet
        ###readout = topTask.createSvc("RootOutputSvc/OutputSvc")
        ###readout.property("OutputStreams").set({"/Event/PSDTools":args.output})
        ##########################################################
        pass

    def add_user_output_vec(self, user_output_vec, args):
        if "USER_OUTPUT" not in user_output_vec:
            user_output_vec.append("USER_OUTPUT")
        if "PSD_OUTPUT" not in user_output_vec:
            user_output_vec.append("PSD_OUTPUT")
