# Particle Identification Using PSDTools 

**PSD** (Pulse Shape Discrimination) use the difference of different particles' emission time profile to
distinguish different particle. **PSDTools** is an offline algorithm which can achieve many PSD methods 
to do the PID work. This doc briefly introduces the principle of these methods and the structure of the algorithm, and provides instructions for users.
More details can be found in JUNO-doc-7640.

## Algorithm Stucture
The algorithm diagram is shown in the figure below. PSDTools composed of three parts which are 
PSDAlg, IPSDInputSvc and IPSDTool. And IPSDTool is the base class which should be inherited by 
other PSD method. 

![Fig. diagram of `PSDTools`](figure/structure.png)

* **Code Location**: ` offline/Reconstruction/PSDTools/`
* **Model Location**: ` data/Reconstruction/....`

## User Interface
PSDTools algorithms are loaded and configured by `tut_calib_rec2PSD.py` and `tut_det2PSD.py`
( `JUNOPSDModule.py` ). `tut_calib_rec2PSD.py` only do the PSD work and take elecsim,calib,rec output files 
as input. And `tut_det2PSD.py` do the elecsim,calib,rec and PSD work at one command which take detsim output 
file as input.

* **Location** : `$PSDTOOLSROOT/share/tut*.py` and `offline/Examples/Tutorial/python/Tutorial/JUNOPSDModule.py` 

Common switches:

* --evtmax
    
    How many events to be executed

* --method-PSD

  Different PSD method can be specific by this switch , so far PSD_TMVA and PSDTool_sklearn methods have been implemented in
  the algorithm.

* --PrepareForTraining
        
    Only do preprocess and save the PSD values so that users can use those output to do the training to get related model.
    For the PID which is not default configured. Users are supposed to train their own model to adapt the related situation.

* --Predict

    Once the related ML model is ready ( if not ready yet, --PrepareForTraining is supposed to be used )
users can use this switch to do the PSD work 

TMVA switches:

* --



* Command example: 
1. `python $PSDTOOLSROOT/share/DetToPSD/tut_calib_rec2PSD.py --evtmax -1 --inputList file_list.txt --user-output user.root –method-PSD PSD_TMVA --Predict`

2. `python $PSDTOOL/share/DetToPSD/tut_det2PSD.py  --evtmax -1 --seed $seed  --disableAfterPulse --input eventinput:detsim.root --rate eventinput:1  --gdml --method energy-point --enableLTSPEs --enableTimeInfo --enableUseEkMap --enableQTimePdf --SaveMC 0 --VtxCorr 0 --elec yes --user-output output.root --method-PSD PSD_TMVA --Predict`

* **note: tut_det2PSD.py should include elecsim, calib and rec options**