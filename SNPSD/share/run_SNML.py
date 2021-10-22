#!/usr/bin/env python
# coding: utf-8
#author: huangxin

import os
import SNML

def get_parser():
    import argparse
    parser=argparse.ArgumentParser(description='run SN PSD')

    parser.add_argument("--input", help="input file name")
    parser.add_argument("--output", default="", help="The user output file")
    parser.add_argument("--model", default="", help="The model used")

    parser.add_argument("--train", dest="usePredict", action="store_false")
    parser.add_argument("--predict", dest="usePredict", action="store_true")
    parser.set_defaults(usePredict = True)

    return parser

if __name__ == "__main__":
    parser=get_parser()
    args=parser.parse_args()

    if args.usePredict:
        theClass = SNML.predict()
        theClass.setInput(args.input)
        theClass.setOutput(args.output)
        theClass.setModel(args.model)
        theClass.classify()
    else:
        theTrain = SNML.train("SNMLP")
        theTrain.setModel(args.model)
        theTrain.loadTrainingData(args.input, 'pES', 'eES')
        theTrain.AddModels()
        #theTrain.CompareModels()
        theTrain.TrainModel('MLPClassifier')
