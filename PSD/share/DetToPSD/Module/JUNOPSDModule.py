# -*- coding:utf-8 -*-
# @Time: 2021/11/25 20:57
# @Author: Luo Xiaojie, Huang Xin
# @Email: luoxj@ihep.ac.cn, huangxin.ihep.ac.cn
# @File: JUNOPSDModule.py

import sys
import os
import Sniper
import logging
from .JUNOModule import JUNOModule
from .JUNOUtils import mh


##############################################################################
# JUNOModule
##############################################################################

class JUNOPSDModule(JUNOModule):

    DATA_LOG_MAP = {"Test":0, "Debug":2, "Info":3, "Warn":4, "Error":5, "Fatal":6}

    def __init__(self):
        pass

    def register_options(self, parser):
        self.register_options_common(parser)
        self.register_options_TMVA(parser)

    def register_options_common(self, parser):
        #---------------input and output-----------------
        parser.add_argument("--inputSvc", default="PSDInputSvc", help="Which PSD input service will be use, default for PSDInputSvc.*")
        parser.add_argument("--pmt_map", default="/cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc830/Pre-Release/J21v1r0-Pre2/data/Simulation/ElecSim/PmtData_Lpmt.root",  help="Map from pmtID to PMT Location")

        #--------------------For PSD--------------------------
        parser.add_argument("--method-PSD", default="TestPSDTool", choices=["TestPSDTool", "PSD_TMVA"], help="The PSD method")
        parser.add_argument("--Predict", dest="usePredict", action="store_true")
        parser.add_argument("--PrepareForTraining", dest="usePredict", action="store_false")
        parser.set_defaults(usePredict = True)

    def register_options_TMVA(self, parser):
        #-------------------For TMVA method-----------------------------
        parser.add_argument("--model_FV1", default="Induction__BDTG.weights_FV1.xml", help="ML model to do the prediction, default is for TMVA method")
        parser.add_argument("--model_FV2", default="Induction__BDTG.weights_FV2.xml", help="ML model to do the prediction, default is for TMVA method")
        parser.add_argument("--R_divide", type=float, default=16, help="radius boundary to divide FV1 and FV2")
        parser.add_argument("--PSD_divide", type=float, default=0., help="Set PSD boundary for bkg and sig, so that tag the event")

    def init(self, toptask, args):
        self.init_common(toptask, args)
        self.init_PSD(toptask, args)

        if args.method_PSD == "PSD_TMVA":
            self.init_TMVA_model(toptask, args)

    def init_common(self, topTask, args):
        topTask.setLogLevel(self.DATA_LOG_MAP[args.loglevel])
        topTask.setEvtMax(args.evtmax)

    def init_PSD(self, topTask, args):
        #=======================PSD related======================
        import PSD
        self.psdalg = topTask.createAlg("PSDAlg")
        self.psdsvc = topTask.createSvc(args.inputSvc)
        self.psdtool = self.psdalg.createTool(args.method_PSD)
        self.psdalg.property("Method").set(args.method_PSD)
        self.psdalg.property("UsePredict").set(args.usePredict)

        self.psdsvc.property("PMT_Map").set(args.pmt_map)

    def init_TMVA_model(self, topTask, args):
        self.psdtool.property("Model_FV1").set(args.model_FV1)
        self.psdtool.property("Model_FV2").set(args.model_FV2)
        self.psdtool.property("R_divide").set(args.R_divide)
        self.psdtool.property("PSD_divide").set(args.PSD_divide)

    def add_output_vec(self, output_vec, args):
        #=======================Output using framwork======================
        ##########################################################
        ###For the output of data model, PSD data model is not implemented yet
        ###readout = topTask.createSvc("RootOutputSvc/OutputSvc")
        ###readout.property("OutputStreams").set({"/Event/PSD":args.output})
        ##########################################################
        pass

    def add_user_output_vec(self, user_output_vec, args):
        if "USER_OUTPUT" not in user_output_vec:
            user_output_vec.append("USER_OUTPUT")
        if "PSD_OUTPUT" not in user_output_vec:
            user_output_vec.append("PSD_OUTPUT")
