#!/usr/bin/env python
# coding: utf-8

import pickle
import os
import ROOT
import uproot as up
import numpy as np
import time
import random
import sys

class SNReader():
    def __init__(self):
        self.Events = None
        return

    def loadDataFromRoot(self, snfile, evtNames=[], enableUseRec=False, enableStore=False):
        #evtNames==[] represents reading all channels, in this case, self.Events=[]
        #can only load data from one file
        T1 = time.time()
        print('Dealing with file:%s'%snfile)
        readAll = (len(evtNames)==0)
        #first define the self.Events
        if readAll:
            self.Events = []
        else:
            self.Events = {}
            for thistype in evtNames:
                self.Events[thistype] = []
        with up.open(snfile) as infile:
            evtTree = infile['evt']
            readin = evtTree.arrays(library='np')
            truthType = readin['truthType']
            totEvts = len(truthType)
            print('Total number of events:%d'%totEvts)
            indices = np.arange(totEvts)
            random.shuffle(indices)
            for ith in indices:
                thistype = truthType[ith]
                if (not readAll) and (thistype not in evtNames):
                    continue
                thisEvent = []
                if enableUseRec:
                    recval = readin['recE'][ith]
                    thisEvent.append(recval)
                for nthVar in range(0, 550):
                    nthvarval = readin['Var%d'%nthVar][ith]
                    thisEvent.append(nthvarval)
                maxval = max(thisEvent)
                if maxval<1:
                    print("Warning: maximum value is smaller than 1!")
                    print(thisEvent)
                    continue
                scaledevent = [i/maxval for i in thisEvent]
                if readAll:
                    self.Events.append(scaledevent)
                else:
                    self.Events[thistype].append(scaledevent)

        T2 = time.time()
        print('Time is:%s s'%((T2-T1)))
        if enableStore:
            outDir='/junofs/users/huangx/MyProject/Supernova/CCSNAnalysis/MLPSD/LS_ML/sk_psd/share'
            filedir = snfile.split('share')
            outfile = outDir+filedir[1].split('.root')[0]+'.npy'
            outfiledir = os.path.dirname(outfile)
            if not os.path.isdir(outfiledir):
                os.makedirs(outfiledir)
            np.save(outfile, self.Events)
        sys.stdout.flush()
        return self.Events

    def loadDataFromNPY(self, snfile, evtNames=[]):
        self.Events = {}
        for snfile in snfiles:
            T1 = time.time()
            filedir = snfile.split('share')
            infile = outDir+filedir[1].split('.root')[0]+'.npy'
            tmpevents = np.load(infile, allow_pickle=True)
            tmpevents = tmpevents.item()
            print(tmpevents.keys())
            #for evttype in tmpevents.keys():
            for evttype in evtNames:
                if evttype not in self.Events.keys():
                    self.Events[evttype]=[]
                for thisevent in tmpevents[evttype]:
                    maxval = max(thisevent)
                    if maxval<1: print('the maximum value is:%d'%maxval)
                    scaledevent = [i/maxval for i in thisevent]
                    self.Events[evttype].append(scaledevent)
                    #self.Events[evttype].append(thisevent)
            T2 = time.time()
            print('Time is:%s ms'%((T2-T1)*1000))

        minlen = 100000000000
        for evttype in evtNames:
            thislen = len(self.Events[evttype])
            if minlen>thislen: minlen = thislen
        for evttype in evtNames:
            self.Events[evttype] = self.Events[evttype][:minlen]
        return self.Events

#if __name__ == "__main__":
#    thisreader = SNReader()
#    thisreader.loadDataFromRoot('/junofs/users/huangx/MyProject/PSD/SNPSD/share/result/SNMLTool/sn_intp2013.data/1/dist10/prepare/prepare_0th.root', ['pES', 'eES'])
