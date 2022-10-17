#!/bin/bash
#export CMTPROJECTPATH=~/scratchfs_juno_500G/PSD_sniper/PSDTools/PSDTools/:$CMTPROJECTPATH
#source /afs/ihep.ac.cn/users/l/luoxj/scratchfs_juno_500G/J21v1r0-Pre2/bashrc
#source /cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc830/Pre-Release/J21v1r0-Pre2/setup.sh
source /afs/ihep.ac.cn/users/l/luoxj/scratchfs_juno_500G/J21v2r0-branch/bashrc

#option=_atm_$(($1+100))
#option=_atm_$1
option=_atm
INPUTFILELIST=fileList$option.txt
#USEROUTPUT=root://junoeos01.ihep.ac.cn//eos/juno/user/luoxj/TestPSDTool/user${option}.root
USEROUTPUT=user${option}.root
LOGFILE=log${option}.txt

METHOD=PSD_TMVA
path_atm=root://junoeos01.ihep.ac.cn//eos/juno/valprod/valprod0/chengjie_J21/AtmNC/Model-G
id=$(printf "%06d\n" $1)
echo $id

(time python $PSDTOOLSROOT/share/DetToPSD/tut_calib_rec2PSD.py --evtmax 20 --input-list  $INPUTFILELIST  --user-output ${USEROUTPUT} --method ${METHOD} --PrepareForTraining  )
#(time python $PSDROOT/share/runPSD.py --evtmax -1 --input $path_atm/ES/elec_sample_$id.root $path_atm/CS/calib_sample_$id.root $path_atm/REC/rec_sample_$id.root   --user-output ${USEROUTPUT} --method ${METHOD} --PrepareForTraining  )
