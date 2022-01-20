# Particle Identification Using PSDTools 

**PSD** (Pulse Shape Discrimination) use the difference of different particles' emission time profile to
distinguish different particle. **PSDTools** is an offline algorithm which can achieve many PSD methods 
to do the PID work. This doc briefly introduces the principle of these methods and the structure of the algorithm, and provides instructions for users.
More details can be found in JUNO-doc-7640.

## Algorithm Stucture
The algorithm diagram is shown in the figure below. PSDTools composed of three parts which are 
PSDAlg, IPSDInputSvc and IPSDTool. And IPSDTool is the base class which should be inherited by 
other PSD method. 

![Fig. diagram of `PSDTools`](figs/Structure.png)

* **Code Location**: ` offline/Reconstruction/PSDTools/`
* **Model Location**: ` data/Reconstruction/PSDTools/`

## User Interface
PSDTools algorithms are loaded and configured by `tut_calib_rec2PSD.py` and `tut_det2PSD.py`
( `JUNOPSDModule.py` ). `tut_calib_rec2PSD.py` only do the PSD work and take elecsim,calib,rec output files 
as input. And `tut_det2PSD.py` do the elecsim,calib,rec and PSD work at one command which take detsim output 
file as input.

* **Location** : `$PSDTOOLSROOT/share/tut*.py` and `offline/Examples/Tutorial/python/Tutorial/JUNOPSDModule.py` 

### Common switches

* `--evtmax`
    
    How many events to be executed

* `--method-PSD`

  Different PSD method can be specific by this switch , so far PSD_TMVA and PSDTool_sklearn methods have been implemented in
  the algorithm.

* `--PrepareForTraining`
        
    Only do preprocess and save the PSD values so that users can use those output to do the training to get related model.
    For the PID which is not default configured. Users are supposed to train their own model to adapt the related situation.

* `--Predict`

    Once the related ML model is ready ( if not ready yet, --PrepareForTraining is supposed to be used )
users can use this switch to do the PSD work 

* `--Model`

    Specify which ML model to be used to do the prediction

* `--PSD-cut`

    Cut to separate signal and background so to set particle tag which should determined by required signal efficiency and background inefficiency.
* `--inputSvc`

    Specify which IPSDInputSvc to use, default is PSDInputSvc. This switch is only for special input requirements.
    
* `--Path_Bins`
    
    Path of file to determine binning strategy for time profile histogram in Sklearn method.File content example:

  ```
  -20, -17, -14, -11, -8, -5, -2, 1, 4, 7, 10, 13, 16, 19, 22, 28, 34, 40, 46, 52, 58, 64, 72, 80, 90, 102, 116, 132, 150, 170, 192, 216, 242, 270, 300, 332, 366, 402, 440, 480, 522, 566, 612, 660, 710, 762,816
  ```

### Command Example: 

1. Simple example [ Only for testing, do nothing ]:
    ```
    python $PSDTOOLSROOT/share/DetToPSD/tut_calib_rec2PSD.py --evtmax -1 --input-list filelist.txt  --user-output output.root --method-PSD TestPSDTool --Predict
    ```
2. TMVA Method [ Only PSD, take elecsim,calib and rec output files as input ]:
   ```
   python $PSDTOOLSROOT/share/DetToPSD/tut_calib_rec2PSD.py --evtmax -1 --input-list  filelist.txt  --user-output output.root --method PSD_TMVA --Predict --Model $JUNOTOP/data/Reconstruction/PSDTools/model/DSNB_AtmNC_10-30MeV_BDTG.weights.xml
   ```

    **filelist.txt:**
   ```
   elecsim.root
   calib.root
   rec.root
   ```
   
3. TMVA Method [ detsim to PSD including elecsim, calib, rec and PSD which take detsim output file as input  ]:
    ```
    python $PSDTOOLSROOT/share/DetToPSD/tut_det2PSD.py  --evtmax -1 --seed 1 \
    --disableAfterPulse --input eventinput:detsim.root --rate eventinput:1  \
   --gdml  --method energy-point --enableLTSPEs --enableTimeInfo --enableUseEkMap --enableQTimePdf --SaveMC 0 --VtxCorr 0 --elec yes \
   --user-output output.root --method-PSD PSD_TMVA --Predict --Model $JUNOTOP/data/Reconstruction/PSDTools/model/DSNB_AtmNC_10-30MeV_BDTG.weights.xml 
    # Second line: elesim options
    # Third line: rec options
    # Fourth line: PSD options
    ```
   
4. Sklearn Method
    ```
    ```
   
  
    
    