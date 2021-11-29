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
# from Tutorial import JUNOGeometryModule
from Tutorial import JUNOMCParameterModule
from Tutorial import JUNOPMTParameterModule
from Tutorial import JUNOPMTCalibSvcModule
from Tutorial import JUNOCondDBModule
from Tutorial import JUNOPSDModule

juno_application = JUNOApplication(description="JUNO Pulse Shape Discrimination")
juno_application.set_default_input("sample_detsim.root")
# juno_application.set_default_output("sample_PSD.root")
juno_application.set_default_user_output("sample_PSD_user.root")

databuffer_module = JUNODataBufferModule()
rootinput_module = JUNORootInputModule()
rootoutput_module = JUNORootOutputModule()
rootwriter_module = JUNORootWriterModule()
# geometry_module = JUNOGeometryModule()
mcparam_module = JUNOMCParameterModule()
pmtparam_module = JUNOPMTParameterModule()
pmtcalib_module = JUNOPMTCalibSvcModule()
conddb_module = JUNOCondDBModule()

PSD_module = JUNOPSDModule.JUNOPSDModule()

juno_application.register(databuffer_module)
juno_application.register(rootinput_module)
juno_application.register(rootoutput_module)
juno_application.register(rootwriter_module)
# juno_application.register(geometry_module)
juno_application.register(mcparam_module)
juno_application.register(pmtparam_module)
juno_application.register(pmtcalib_module)
juno_application.register(conddb_module)

juno_application.register(PSD_module)

juno_application.run()