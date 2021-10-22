#!/usr/bin/env python
# coding: utf-8

import warnings

from sklearn.exceptions import ConvergenceWarning
import numpy as np

import pickle, sys
import ROOT
import uproot as up
from scipy.special import softmax
import random
import load_SN
from array import array

class train():
    def __init__(self, task):
        self.TaskType = task
        self.input_train = []
        self.target_train= []
        self.modelName = 'default.pkl'

    def setModel(self, modelName):
        self.modelName = modelName

    def loadTrainingData(self, snFileList, sigName, bkgName):
        #read the file list to get input file names
        snfiles=[]
        with open(snFileList) as thisfile:
            for line in thisfile:
                line=line.strip()
                snfiles.append(line)
        #totFiles = len(snfiles)
        #snfiles = snfiles[:totFiles/2]
        snfiles = snfiles[:100]
        print(snfiles)
        #load the data from root file.
        sigEvents = []
        bkgEvents = []
        snreader = load_SN.SNReader()
        for snfile in snfiles:
            snEvents = snreader.loadDataFromRoot(snfile, [sigName, bkgName])
            sigEvents += snEvents[sigName]
            bkgEvents += snEvents[bkgName]
        #snEvents = snreader.loadDataFromNPY(snfiles, [sigName, bkgName])
        #sigEvents = snEvents[sigName]
        #bkgEvents = snEvents[bkgName]
        print('Number of events:%d, %d'%(len(sigEvents), len(bkgEvents)))
        #Tag the events with 0 for signal and 1 for background
        sigTags = np.zeros(len(sigEvents))
        bkgTags = np.ones(len(bkgEvents))
        #merge neutron and gamma datasets to make one single dataset.
        events = np.concatenate((sigEvents, bkgEvents) , axis=0)
        tags = np.concatenate((sigTags, bkgTags))
        #shuffle the signal and background events by indices
        indices = np.arange(len(events))
        random.shuffle(indices)
        #split the dataset into two parts for training and testing respectively.
        for index in indices:
            self.input_train.append(events[index])
            self.target_train.append(tags[index])

    def AddModels(self):
        #Add multiple models for comparison, look for the definition\
        #of each model on sklearn documentation
        from sklearn.linear_model import LogisticRegression
        from sklearn.svm import SVC, LinearSVC
        from sklearn.neighbors import KNeighborsClassifier
        from sklearn.tree import DecisionTreeClassifier
        from sklearn.ensemble import RandomForestClassifier
        from sklearn.naive_bayes import GaussianNB
        from sklearn.neural_network import MLPClassifier
        from sklearn.naive_bayes import GaussianNB

        models = {}

        ##models["LogisticRegression"]  = LogisticRegression()
        #models["SVC"] = SVC()
        ##models["LinearSVC"] = LinearSVC()
        #models["KNeighbors"] = KNeighborsClassifier()
        #models["DecisionTree"] = DecisionTreeClassifier()
        #models["RandomForest"] = RandomForestClassifier()
        #rf2 = RandomForestClassifier(n_estimators=100, criterion='gini',
        #                                        max_depth=10, random_state=0, max_features=None)
        #models["RandomForest2"] = rf2
        #models["MLPClassifier"] = MLPClassifier(solver='lbfgs', random_state=0, verbose=True)
        models["MLPClassifier"] = MLPClassifier(solver='adam', max_iter=1000, random_state=0, verbose=True)#
        #models["GaussianNB"] =  GaussianNB()
        self.models = models

    def CompareModels(self):
        #compare the efficiency of different models.
        results = []
        names = []
        models = self.models
        from sklearn.model_selection import cross_val_score
        for name in models.keys():
            model = models[name]
            result = cross_val_score(model, self.input_train, self.target_train)
            names.append(name)
            results.append(result)
        for i in range(len(names)):
            print(names[i],results[i].mean())

    def TrainModel(self, modelname):
        #Train and save a specific model.
        if modelname not in self.models.keys():
            print('Model not supported yet!')
            sys.exit()
        else:
            print('Train model %s' % modelname)
            classifier = self.models[modelname]
            classifier.fit(self.input_train, self.target_train)
            #pkl_filename = 'model_%s.pkl'%self.TaskType
            with open(self.modelName, 'wb') as pfile:
                pickle.dump(classifier, pfile)
            input_test = self.input_test
            target_test = self.target_test
            if False:
                predict_proba = classifier.predict_proba(input_test)
                #print(predict_proba)
                print(predict_proba.shape)
                print(classifier.predict(input_test[:10]), target_test[:10])
        sys.stdout.flush()

class predict():
    def __init__(self):
        self.infileName = ''
        self.outfileName = ''
        self.modelName = ''

        self.model = None
        self.Events = None

        self.outputFile = None
        self.outputTree = None
        self.psdVal = array('d', [-1])
        return

    def setInput(self, infileName):
        self.infileName = infileName

    def setOutput(self, outfileName):
        self.outfileName = outfileName

    def setModel(self, modelName):
        self.modelName = modelName

    def loadModel(self):
        with open(self.modelName, 'rb') as mfile:
            self.model = pickle.load(mfile)

    def loadData(self):
        snreader = load_SN.SNReader()
        self.Events = snreader.loadDataFromRoot(self.infileName)

    def initOutput(self):
        evtTypes = ["IBD", "IBDp", "IBDd", "pES", "eES", "C12", "N12", "B12", "AfterPulse", "pileUp"]
        self.outputFile = ROOT.TFile.Open(self.outfileName, 'RECREATE')
        self.outputTree = ROOT.TTree('PSDVal', 'PSDVal')
        self.outputTree.Branch('psdval', self.psdVal, 'psdval/D')

    def classify(self):
        self.initOutput()
        self.loadData()
        self.loadModel()
        #calculate the psd value
        psdval = self.model.predict_proba(np.array(self.Events))
        totEvts = len(self.Events)
        for ith in range(0, totEvts):
            self.psdVal[0] = psdval[ith][0]
            self.outputTree.Fill()
        self.outputFile.Write()
