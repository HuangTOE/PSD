#!/bin/bash
#export CMTPROJECTPATH=~/scratchfs_juno_500G/PSD_sniper/PSDTools/PSDTools/:$CMTPROJECTPATH
#source /afs/ihep.ac.cn/users/l/luoxj/scratchfs_juno_500G/J21v1r0-Pre2/bashrc
#source /cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc830/Pre-Release/J21v1r0-Pre2/setup.sh
source /afs/ihep.ac.cn/users/l/luoxj/scratchfs_juno_500G/J21v2r0-branch/bashrc
#
#source ../cmt/setup.sh

option=_atm
INPUTFILELIST=fileList$option.txt
USEROUTPUT=user${option}_with_FV2.root
LOGFILE=log.txt

#METHOD=TestPSDTool
#time python runPSD.py --evtmax 10 --inputList ${INPUTFILELIST} --user-output ${USEROUTPUT} --method ${METHOD} --enablePredict
#
METHOD=PSD_TMVA
#(time python runPSD.py --evtmax 10 --inputList ${INPUTFILELIST} --user-output ${USEROUTPUT} --method ${METHOD} --disablePredict) >& ${LOGFILE}
#(time python runPSD.py --evtmax 10 --inputList ${INPUTFILELIST} --user-output ${USEROUTPUT} --method ${METHOD} --enablePredict )
(time python runPSD.py --evtmax 10 --input a1.root a2.root a3.root --user-output ${USEROUTPUT} --method ${METHOD} --enablePredict )

