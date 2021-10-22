#!/usr/bin/env python
#-*- coding: utf-8 -*-

import uproot as up
import numpy as np
import matplotlib.pyplot as plt
plt.style.use('HXStyle')

class ana_ML():
    def __init__(self):
        self.evtType_truth = []
        self.recE_truth = []
        self.vtx_truth = [] #(r, x, y, z)
        self.psdVal = []

        #figures
        self.figs = []

    def reset(self):
        self.evtType_truth = []
        self.recE_truth = []
        self.vtx_truth = [] #(r, x, y, z)
        self.psdVal = []

    def readTruth(self, truthFileName):
        print('Reading truth file...')
        with up.open(truthFileName) as truthFile:
            truthTree = truthFile['SNTruth']
            readin = truthTree.arrays(library='np')
            self.evtType_truth += list(readin['evtType'])
            self.recE_truth += list(readin['recE'])

    def readPSDPredict(self, psdFileName):
        print('Reading PSD results...')
        with up.open(psdFileName) as psdFile:
            psdTree = psdFile['PSD']
            readin = psdTree.arrays(library='np')
            self.psdVal += list(readin['psdVal'])

    def drawPSDVal(self, evtNames, ax=None):
        if ax==None:
            fig, ax=plt.subplots()
            self.figs.append(fig)
        psdval = {}
        for thistype in evtNames:
            psdval[thistype] = []
        totEvts = len(self.evtType_truth)
        for ith in range(0, totEvts):
            thistype = self.evtType_truth[ith]
            if thistype not in evtNames: continue
            psdval[thistype].append(self.psdVal[ith])

        for thistype in evtNames:
            ax.hist(psdval[thistype], histtype='step', bins=50, density=True, label=thistype, linewidth=3)
        ax.set(yscale='log')
        ax.legend()

        return ax

if __name__ == "__main__":
    fileNo = 0
    thisana = ana_ML()
    for fileNo in range(3, 53):
        thisana.readTruth('/junofs/users/huangx/MyProject/Supernova/CCSNAnalysis/SNTruth/share/result/sn_intp2013.data/1/dist10/sn_%dth.root'%fileNo)
        thisana.readPSDPredict('/junofs/users/huangx/MyProject/PSD/SNPSD/myJob/result/SNMLTool/sn_intp2013.data/1/dist10/predict/predict_%dth.root'%fileNo)
    ax = thisana.drawPSDVal(['AfterPulse'])#, 'eES', 'AfterPulse'

    thisana.reset()
    for fileNo in range(103, 153):
        thisana.readTruth('/junofs/users/huangx/MyProject/Supernova/CCSNAnalysis/SNTruth/share/result/sn_intp2013.data/1/dist10/sn_%dth.root'%fileNo)
        thisana.readPSDPredict('/junofs/users/huangx/MyProject/PSD/SNPSD/myJob/result/SNMLTool/sn_intp2013.data/1/dist10/predict/predict_%dth.root'%fileNo)
    thisana.drawPSDVal(['pES'], ax)#, 'eES', 'AfterPulse'

    plt.show()
