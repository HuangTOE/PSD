# -*- coding:utf-8 -*-
# @Time: 2021/11/25 20:57
# @Author: Luo Xiaojie, Huang Xin
# @Email: luoxj@ihep.ac.cn, huangxin.ihep.ac.cn
# @File: tut_calib_rec2PSD.py

from Tutorial import JUNOApplication
from Tutorial import JUNODataBufferModule
from Tutorial import JUNORootWriterModule
from Tutorial import JUNORootInputModule
from Tutorial import JUNORootOutputModule
from Tutorial import JUNOGeometryModule
from Tutorial import JUNOMCParameterModule
from Tutorial import JUNOPMTParameterModule
from Tutorial import JUNOPMTCalibSvcModule
from Tutorial import JUNOCondDBModule
from Tutorial import JUNOPSDModule
# from Tutorial import JUNORootInputModule # Note: ElecSim implements its own input
from Tutorial import JUNOElecSimModule
from Tutorial import JUNOWaveRecModule
from Tutorial import JUNOEventRecModule

##############################################################################
# flag to control:
# * waveform reconstruction (--waverec or --no-waverec)
# * event reconstruction    (--evtrec  or --no-evtrec)
##############################################################################

FLAG_WAVEREC = True
FLAG_EVTREC = True
import sys
if '--no-waverec' in sys.argv:
    FLAG_WAVEREC = False
    FLAG_EVTREC = False
    sys.argv.remove('--no-waverec') # disable waveform rec.
if '--no-evtrec' in sys.argv:
    FLAG_EVTREC = False
    sys.argv.remove('--no-evtrec')  # disable event rec.

juno_application = JUNOApplication(description="JUNO Electronics Simulation && Waveform Reconstruction && Event Reconstruction && PSD")
juno_application.set_default_input_action("append")
# juno_application.set_default_output("sample_PSD.root")
juno_application.set_default_user_output("sample_PSD_user.root")

databuffer_module = JUNODataBufferModule()
# rootinput_module = JUNORootInputModule()
rootwriter_module = JUNORootWriterModule()
geometry_module = JUNOGeometryModule()
mcparam_module = JUNOMCParameterModule()
pmtparam_module = JUNOPMTParameterModule()
pmtcalib_module = JUNOPMTCalibSvcModule()
conddb_module = JUNOCondDBModule()

#########################################################################
# To Enable EDM output, uncomment this code block
# rootoutput_module = JUNORootOutputModule()
# rootoutput_module.unregister_output_key("/Event/Elec") # disable the waveform
# rootoutput_module.unregister_output_key("/Event/Calib") # disable the calib output
# rootoutput_module.unregister_output_key("/Event/Rec") # disable the rec output
# rootoutput_module.unregister_output_key("/Event/Sim") # disable the simulation output
# rootoutput_module.unregister_output_key("/Event/Gen") # disable the generator output
# juno_application.register(rootoutput_module)
#########################################################################


elecsim_module = JUNOElecSimModule()
waverec_module = JUNOWaveRecModule()
eventrec_module = JUNOEventRecModule()
PSD_module = JUNOPSDModule.JUNOPSDModule()

juno_application.register(databuffer_module)
# juno_application.register(rootinput_module) # this module will cause elecsim input error!
juno_application.register(rootwriter_module)
juno_application.register(geometry_module)
juno_application.register(mcparam_module)
juno_application.register(pmtparam_module)
juno_application.register(pmtcalib_module)
juno_application.register(conddb_module)

juno_application.register(elecsim_module)

if FLAG_WAVEREC:
    juno_application.register(waverec_module)
if FLAG_WAVEREC and FLAG_EVTREC:
    juno_application.register(eventrec_module)

juno_application.register(PSD_module)

juno_application.run()