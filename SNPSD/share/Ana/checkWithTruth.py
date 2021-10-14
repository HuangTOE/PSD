#!/usr/bin/env python
#-*- coding: utf-8 -*-

class PSDCheck():
    def __init__(self):
        pass

    def getPSDVal(self, ithTrail, ERange):
        lPSDVal={}
        lPSDVal['all']=[]
        lrecE={}
        lrecE['all']=[]
        lrecR={}
        lrecR['all']=[]
        import ROOT
        from array import array
        SNTYPE='sn'
        MODEL='intp2013.data'
        DISTANCE=10
        MO=1

        METHOD='BDT'

        #read the PSD result
        topDir='/junofs/users/huangx/MyProject/Supernova/CCSNAnalysis/SNPSD/share/result/%s_%s/%d/dist%d'%(SNTYPE, MODEL, MO,  DISTANCE)
        inputFileName=topDir+'/Appl_%s_%dth.root'%(METHOD, ithTrail)
        inputFile=ROOT.TFile(inputFileName, 'READ')
        tree_PSDResult=inputFile.Get('PSDResult')
        totEvts=tree_PSDResult.GetEntries()
        print('total number events:%d'%totEvts)
        psdval=array('d', [0])
        recE=array('d', [0])
        evttype=ROOT.std.string()
        tree_PSDResult.SetBranchAddress('PSDVal', psdval)
        tree_PSDResult.SetBranchAddress('evtType', evttype)
        tree_PSDResult.SetBranchAddress('recE', recE)

        #read the truth info
        topDir='/junofs/users/huangx/MyProject/Supernova/CCSNAnalysis/SNTruth/share/result/%s_%s/%d/dist%d'%(SNTYPE, MODEL, MO,  DISTANCE)
        truthFileName=topDir+'/%s_%dth.root'%(SNTYPE, ithTrail)
        truthFile=ROOT.TFile(truthFileName, 'READ')
        tree_truth=truthFile.Get('SNTruth')
        evttype_truth=ROOT.std.string()
        recE_truth=array('f', [0])
        recX_truth=array('d', [0])
        recY_truth=array('d', [0])
        recZ_truth=array('d', [0])
        tree_truth.SetBranchAddress('evtType', evttype_truth)
        tree_truth.SetBranchAddress('recE', recE_truth)
        tree_truth.SetBranchAddress('recX', recX_truth)
        tree_truth.SetBranchAddress('recY', recY_truth)
        tree_truth.SetBranchAddress('recZ', recZ_truth)

        for ith in range(0, totEvts):
            tree_PSDResult.GetEntry(ith)
            tree_truth.GetEntry(ith)
            if recE_truth[0]>ERange[1]:continue
            if recE_truth[0]<ERange[0]:continue
            import math
            rr=math.sqrt(recX_truth[0]**2+recY_truth[0]**2+recZ_truth[0]**2)
            rr/=1000.
            if rr>16: continue
            #if evttype_truth=="IBDp":
            #    if recE_truth[0]>10 and recE_truth[0]<11:
            #        print("%.3fMeV:%d"%(recE_truth[0], ith))
            #        if psdval[0]>0: print("%dth:%.3f"%(ith, psdval[0]))
            lPSDVal['all'].append(psdval[0])
            lrecE['all'].append(recE_truth[0])
            lrecR['all'].append(recE_truth[0])
            truthType=str(evttype_truth)
            if not truthType in lPSDVal.keys():
                lPSDVal[truthType]=[]
                lrecE[truthType]=[]
                lrecR[truthType]=[]
            lPSDVal[truthType].append(psdval[0])
            lrecE[truthType].append(recE_truth[0])
            lrecR[truthType].append(rr**3)

        return lPSDVal, lrecE, lrecR

    def getTimePDF(self, evtType):
        lhittime=[]
        lhitcharge=[]
        lrecE=[]
        import ROOT
        from array import array
        SNTYPE='sn'
        MODEL='intp2013.data'
        DISTANCE=10
        MO=1

        #read the PSD result
        topDir='/junofs/users/huangx/MyProject/Supernova/CCSNAnalysis/SNPSD/share/result/%s_%s/%d/dist%d'%(SNTYPE, MODEL, MO,  DISTANCE)
        inputFileName=topDir+'/result_0th.root'
        inputFile=ROOT.TFile(inputFileName, 'READ')
        tree_PSDResult=inputFile.Get('PSDResult')
        totEvts=tree_PSDResult.GetEntries()
        print('total number events:%d'%totEvts)
        psdval=array('d', [0])
        thishittime=ROOT.std.vector("double")()
        thishitcharge=ROOT.std.vector("double")()
        tree_PSDResult.SetBranchAddress('PSDVal', psdval)
        tree_PSDResult.SetBranchAddress('hittime', thishittime)
        tree_PSDResult.SetBranchAddress('hitcharge', thishitcharge)

        #read the truth info
        topDir='/junofs/users/huangx/MyProject/Supernova/CCSNAnalysis/SNTruth/share/result/%s_%s/%d/dist%d'%(SNTYPE, MODEL, MO,  DISTANCE)
        truthFileName=topDir+'/%s_0th.root'%SNTYPE
        truthFile=ROOT.TFile(truthFileName, 'READ')
        tree_truth=truthFile.Get('SNTruth')
        evttype_truth=ROOT.std.string()
        recE_truth=array('f', [0])
        tree_truth.SetBranchAddress('evtType', evttype_truth)
        tree_truth.SetBranchAddress('recE', recE_truth)

        #get tree content
        totN=0;
        for ith in range(0, totEvts):
            tree_truth.GetEntry(ith)
            if evttype_truth!=evtType: continue
            tree_PSDResult.GetEntry(ith)
            lrecE.append(recE_truth[0])
            print(recE_truth[0])
            lhittime.append(list(thishittime))
            lhitcharge.append(list(thishitcharge))

            totN+=1
            if totN>20: break
        print('evtType:%s'%evttype_truth)

        return lrecE, lhittime, lhitcharge

def drawPSDVal():
    thecheck=PSDCheck()
    evtType=['pES', 'eES']#, 'AfterPulse'
    #evtType=['all', 'eES', 'pES', 'IBDp', 'IBDd', 'AfterPulse']

    lpsdval={}
    lrecE={}
    lrecR={}
    for thistype in evtType:
        lpsdval[thistype]=[]
        lrecE[thistype]=[]
        lrecR[thistype]=[]

    for ithTrail in [0, 2, 3, 4]: #range(0, 1):
        tmplpsdval, tmplrecE, tmplrecR=thecheck.getPSDVal(ithTrail, [0, 1])
        for thistype in evtType:
            lpsdval[thistype]+=tmplpsdval[thistype]
            lrecE[thistype]+=tmplrecE[thistype]
            lrecR[thistype]+=tmplrecR[thistype]

    import matplotlib.pyplot as plt
    plt.style.use('HXStyle')
    fig, ax=plt.subplots()
    fig2, ax2=plt.subplots()
    fig3, ax3=plt.subplots()
    for thistype in evtType:
        ax.hist(lpsdval[thistype], histtype='step', bins=50, range=[-0.35, 0.35], label=thistype, linewidth=6)#range=[-0.06, 0.06]
        ax2.scatter(lrecE[thistype], lpsdval[thistype], label=thistype, s=5)
        ax3.scatter(lrecR[thistype], lpsdval[thistype], label=thistype, s=5)
        #ax2.hist2d(lrecE[thistype], lpsdval[thistype], bins=[50, 50], cmap=plt.cm.Spectral_r, cmin =1)
        #ax3.hist2d(lrecR[thistype], lpsdval[thistype], bins=[50, 50], cmap=plt.cm.Spectral_r, cmin =1)
    ax.legend()
    ax.set(xlabel='PSD Value', ylabel='a.u.', yscale='log')
    ax2.legend()
    ax2.set(xlabel='$E_{rec}$ [MeV]', ylabel='PSD Value', xscale='log')
    ax3.legend()
    ax3.set(xlabel='$R^{3}$ [$m^{3}$]', ylabel='PSD Value',)

    plt.show()
    return

def drawROC():
    thecheck=PSDCheck()
    sigName='pES'
    bkgName='eES'
    evtType=[sigName, bkgName]

    lpsdval={}
    lrecE={}
    lrecR={}
    for thistype in evtType:
        lpsdval[thistype]=[]
        lrecE[thistype]=[]
        lrecR[thistype]=[]
    for ithTrail in [0, 2, 3, 4]: #range(0, 1):
        tmplpsdval, tmplrecE, tmplrecR=thecheck.getPSDVal(ithTrail, [0, 10000])
        for thistype in evtType:
            lpsdval[thistype]+=tmplpsdval[thistype]
            lrecE[thistype]+=tmplrecE[thistype]
            lrecR[thistype]+=tmplrecR[thistype]

    import numpy as np
    roc_sig_efficiency=[]
    roc_bkg_rejection=[]
    roc_cut=np.linspace(-0.03, 0.05, 1000)
    for thiscut in roc_cut:
        sigeff=0.
        for sig_psdval in lpsdval[sigName]:
            if sig_psdval>thiscut: sigeff+=1
        sigeff/=len(lpsdval[sigName])
        roc_sig_efficiency.append(sigeff)

        bkgrej=0.
        for bkg_psdval in lpsdval[bkgName]:
            if bkg_psdval<thiscut: bkgrej+=1
        bkgrej/=len(lpsdval[bkgName])
        roc_bkg_rejection.append(bkgrej)

    import matplotlib.pyplot as plt
    plt.style.use('HXStyle')
    fig, ax=plt.subplots()
    #print(roc_sig_efficiency)
    #print(roc_bkg_rejection)
    ax.plot(roc_sig_efficiency, roc_bkg_rejection)
    ax.set(xlabel='signal efficiency', ylabel='background rejection', xlim=[0, 1], ylim=[0, 1])
    ax.grid()

    plt.show()
    return

def drawTimePDF():
    thecheck=PSDCheck()
    lrecE, lhittime, lhitcharge=thecheck.getTimePDF('eES')

    import matplotlib.pyplot as plt
    plt.style.use('HXStyle')
    fig={}
    ax={}
    for ith in range(0, len(lhittime)):
        fig[ith], ax[ith]=plt.subplots()
        ax[ith].hist(lhittime[ith], histtype='step', bins=550, range=[-100, 1000], linewidth=1, label='reE:%.2f'%lrecE[ith])
        ax[ith].set(yscale='log')
        ax[ith].legend()

    plt.show()

if __name__ == "__main__":
    #drawTimePDF()
    drawPSDVal()
    #drawROC()
