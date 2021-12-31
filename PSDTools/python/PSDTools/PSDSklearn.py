#!/usr/bin/env python
#-*- coding: utf-8 -*-

from Sniper import PyAlgBase


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
        self.h_time = None

    def initialize(self):
        self.datastore = self.get("DataStore").data()

        return True

    def execute(self):

        # Get pmtid/npe/hittime
        self.h_time = self.access_array("h_time")

        if self.h_time is None:
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
        import ExamplePyAlg
        if hasattr(ExamplePyAlg, name):
            print("Get %s from module ExamplePyAlg"%name)
            return getattr(ExamplePyAlg, name)
        elif "pmtid" in self.datastore:
            print("Get %s from PyDataStore"%name)
            return self.datastore[name]
        else:
            print("Can't get %s in the store."%name)
            print(self.datastore)

    def process_data_in_numpy(self):
        print("h_time:\t",self.h_time)
        # calculate the sum of nPE
        totalpe = np.sum(self.h_time)
        print("Total PE (numpy): ", totalpe)

        # calculate the mean of hittime
        meanhittime = np.mean(self.h_time)
        print("Mean hit time (numpy): ", meanhittime)



