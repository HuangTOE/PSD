#!/bin/bash
cd /junofs/users/huangx/MyProject
source OECsetup.sh
source /junofs/users/huangx/MyProject/PSD/PSD/cmt/setup.sh

cd /junofs/users/huangx/MyProject/PSD/PSD/share

INPUTFILELIST=fileList.txt
USEROUTPUT=user.root
LOGFILE=log.txt

METHOD=TestPSDTool

(time python runPSD.py --evtmax 10 --inputList ${INPUTFILELIST} --user-output ${USEROUTPUT} --method ${METHOD} --enablePredict) >& ${LOGFILE}
