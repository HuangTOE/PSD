#!/usr/bin/env python
#-*- coding: utf-8 -*-

from Sniper import PyAlgBase
import pickle
from sklearn.neural_network import MLPClassifier


##############################################################################
# Numpy is installed by default in JUNO offline software
##############################################################################
try:
    import numpy as np
except:
    np = None


class PSDSklearn(PyAlgBase):

    def __init__(self, name):
        PyAlgBase.__init__(self, name)

    def initialize(self):
        self.datastore = self.get("DataStore").data()
        self.model = None

        return True

    def execute(self):
        if self.model == None:
            self.load_model(self.datastore["path_model"])

        # Get Time Profile for PSD
        self.h_time_with_charge = self.access_array("h_time_with_charge")
        self.h_time_without_charge = self.access_array("h_time_without_charge")
        self.xyz_E = self.access_array("xyz_E")

        if (self.h_time_with_charge is None) or (self.h_time_without_charge is None):
            print("array h_time not found. ")
            return False

        if np:
            self.process_data_in_numpy()


        return True

    def finalize(self):
        return True

    ##########################################################################
    # HELPERS
    ##########################################################################
    def access_array(self, name):
        import PSDTools.PSDSklearn
        try:
            return self.datastore[name]
        except:
            print("Can't get %s in the store."%name)
            print(self.datastore)

    # if hasattr(PSDTools.PSDSklearn, name):
        #     print("Get %s from module ExamplePyAlg"%name)
        #     return getattr(PSDTools.PSDSklearn, name)
        # elif "pmtid" in self.datastore:
        #     print("Get %s from PyDataStore"%name)
        #     return self.datastore[name]
        # else:
        #     print("Can't get %s in the store."%name)
        #     print(self.datastore)

    def process_data_in_numpy(self):
        print("h_time_with_charge:\t",self.h_time_with_charge)
        print("h_time_without_charge:\t",self.h_time_without_charge)
        print("xyz_E:\t", self.xyz_E)

        print("Predict:\t", self.model.predict_proba([np.concatenate((self.h_time_without_charge/np.max(self.h_time_without_charge),
                                                                      self.h_time_with_charge/np.max(self.h_time_with_charge)))]))

    def load_model(self, name_model):
        print(f"Loading Sklearn Model {name_model} .........")
        with open(name_model, "rb") as f_model:
            self.model = pickle.load(f_model)





