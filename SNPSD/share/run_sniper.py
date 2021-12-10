#!/usr/bin/env python
#-*- coding: utf-8 -*-
#author: huangxin

import Sniper
import os

def get_parser():
    import argparse
    parser=argparse.ArgumentParser(description='run SN PSDTools')

    #---------------log level-----------------
    parser.add_argument("--evtmax", type=int, default=-1, help='events to be processed')
    parser.add_argument("--loglevel", default="Debug", choices=["Test", "Debug", "Info", "Warn", "Error", "Fatal"],help="Set the Log Level")

    #---------------input and output-----------------
    parser.add_argument("--input", help="input file name")
    parser.add_argument("--inputList", action="append", help="input file list name, this file list contains the names of input files")
    parser.add_argument("--output", default="", help="The user output file")
    parser.add_argument("--user-output", default="", help="The user output file")
    parser.add_argument("--truthFile", default="", help="the truth file")
    parser.add_argument("--weightFile", default="", help="the truth file")

    #--------------------For PSDTools--------------------------
    parser.add_argument("--method", default="TestPSDTool", choices=["TestPSDTool", "SNMLTool"], help="The PSDTools method")
    parser.add_argument("--enablePredict", dest="usePredict", action="store_true")
    parser.add_argument("--disablePredict", dest="usePredict", action="store_false")
    parser.set_defaults(usePredict = True)

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
        filelist.append(args.input)
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
    ###For the output of data model, PSDTools data model is not implemented yet
    ###readout = topTask.createSvc("RootOutputSvc/OutputSvc")
    ###readout.property("OutputStreams").set({"/Event/PSDTools":args.output})
    ##########################################################
    import RootWriter
    rootwriter = topTask.createSvc("RootWriter")
    userOutputMap = {"USER_OUTPUT":args.user_output, "PSD_OUTPUT":args.user_output}
    rootwriter.property("Output").set(userOutputMap)

    #=======================PSDTools related======================
    import PSD
    psdalg = topTask.createAlg("PSDAlg")
    if args.method == 'SNMLTool':
        import SNPSD
        psdtool = psdalg.createTool(args.method)
        psdtool.property("truthFile").set(args.truthFile)
        psdtool.property("weightFile").set(args.weightFile)
    psdalg.property("Method").set(args.method)
    psdalg.property("UsePredict").set(args.usePredict)

    topTask.show()
    topTask.run()
