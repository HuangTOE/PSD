#!/usr/bin/env python
#-*- coding: utf-8 -*-
#author: huangxin

import Sniper
import os

def get_parser():
    import argparse
    parser=argparse.ArgumentParser(description='run SN PSD')

    #---------------log level-----------------
    parser.add_argument("--evtmax", type=int, default=-1, help='events to be processed')
    parser.add_argument("--loglevel", default="Debug", choices=["Test", "Debug", "Info", "Warn", "Error", "Fatal"],help="Set the Log Level")

    #---------------input and output-----------------
    parser.add_argument("--input", nargs="+" ,help="input list of files name")
    parser.add_argument("--inputList", action="append", help="input file list name, this file list contains the names of input files")
    parser.add_argument("--inputSvc", default="PSDInputSvc", help="Which PSD input service will be use, default for PSDInputSvc.*")
    parser.add_argument("--pmt_map", default="/cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc830/Pre-Release/J21v1r0-Pre2/data/Simulation/ElecSim/PmtData_Lpmt.root",  help="Map from pmtID to PMT Location")
    parser.add_argument("--output", default="", help="The user output file")
    parser.add_argument("--user-output", default="", help="The user output file")

    #--------------------For PSD--------------------------
    parser.add_argument("--method", default="TestPSDTool", choices=["TestPSDTool", "PSD_TMVA"], help="The PSD method")
    parser.add_argument("--Predict", dest="usePredict", action="store_true")
    parser.add_argument("--Train", dest="usePredict", action="store_false")
    parser.set_defaults(usePredict = True)

    #-------------------For TMVA method-----------------------------
    parser.add_argument("--model_FV1", default="Induction__BDTG.weights_FV1.xml", help="ML model to do the prediction, default is for TMVA method")
    parser.add_argument("--model_FV2", default="Induction__BDTG.weights_FV2.xml", help="ML model to do the prediction, default is for TMVA method")
    parser.add_argument("--R_divide", type=float, default=16, help="radius boundary to divide FV1 and FV2")
    parser.add_argument("--PSD_divide", type=float, default=0., help="Set PSD boundary for bkg and sig, so that tag the event")

    return parser

DATA_LOG_MAP = {"Test":0, "Debug":2, "Info":3, "Warn":4, "Error":5, "Fatal":6}

if __name__ == "__main__":
    parser=get_parser()
    args=parser.parse_args()

    topTask=Sniper.TopTask("TopTask")
    topTask.setLogLevel(DATA_LOG_MAP[args.loglevel])
    topTask.setEvtMax(args.evtmax)

    #=======================Input using framework======================
    filelist=[]

    if args.input:
        for input in args.input:
            filelist.append(input)
    if args.inputList:
        for fname in args.inputList:
            with open(fname) as hxf:
                for line in hxf:
                    line=line.strip()
                    filelist.append(line)
    import BufferMemMgr
    bufmgr=topTask.createSvc("BufferMemMgr")
    bufmgr.property("StopTaskOpt").set(1)
    #bufmgr.property("TimeWindow").set([-10,10])
    import RootIOSvc
    readin=topTask.createSvc("RootInputSvc/InputSvc")
    readin.property("InputFile").set(filelist)

    #=======================Output using framwork======================
    ##########################################################
    ###For the output of data model, PSD data model is not implemented yet
    ###readout = topTask.createSvc("RootOutputSvc/OutputSvc")
    ###readout.property("OutputStreams").set({"/Event/PSD":args.output})
    ##########################################################
    import RootWriter
    rootwriter = topTask.createSvc("RootWriter")
    #userOutputMap = {"USER_OUTPUT":args.user_output, "PSD_OUTPUT":"tmp.root"}
    userOutputMap = { "PSD_OUTPUT":args.user_output,"USER_OUTPUT":args.user_output}
    rootwriter.property("Output").set(userOutputMap)

    #=======================PSD related======================
    import PSD
    psdalg = topTask.createAlg("PSDAlg")
    psdsvc = topTask.createSvc(args.inputSvc)
    psdtool = psdalg.createTool(args.method)
    psdalg.property("Method").set(args.method)
    psdalg.property("UsePredict").set(args.usePredict)

    psdsvc.property("PMT_Map").set(args.pmt_map)

    # psdtool.property("Model").set("Model")
    if args.method == "PSD_TMVA":
        psdtool.property("Model_FV1").set(args.model_FV1)
        psdtool.property("Model_FV2").set(args.model_FV2)
        psdtool.property("R_divide").set(args.R_divide)
        psdtool.property("PSD_divide").set(args.PSD_divide)

    topTask.show()
    topTask.run()
