find_package(Boost COMPONENTS numpy)
PKG(PSDTools
    DEPENDS
        SimEvent
        Boost::numpy
        ElecEvent 
        CalibEvent 
        RecEvent 
        EvtNavigator 
        BufferMemMgr
        Identifier
        RootIOSvc      
        TMVA
        RooFit
        RooFitCore
        Minuit2
        Geometry
)
