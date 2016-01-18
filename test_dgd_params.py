#!/usr/bin/python
import sys
import os
import subprocess
from subprocess import Popen, PIPE

arguments = {}

if(len(sys.argv) < 2):
  print("Usage : python run_config.py <executable> <config_file> <plot or not>")
  sys.exit()

plot_script="plot_qr.py"

f=open(sys.argv[2], 'r')
for line in f:
  if(len(line) < 3):
    continue
  str1=(line.strip()).split('=')
  arg_key=str1[0]
  if(arg_key[0] == '#'):
    continue;
  arg_val=str1[1]
  if(arg_key == "plot_script"):
    plot_script=arg_val
    continue;
  arguments[arg_key] = arg_val

#print(arguments)
orig_prefix=arguments["prefix"]
for pupdate_time in (0.0001, 0.00005):
	if(pupdate_time == 0.00005):
		continue
	for dalpha in (0.2, 0.3):
		for dgamma in (1.0, 10.0):
			for dgm in (0.0000000001, 0.000000001):
                		arguments["price_update_time"] = str(pupdate_time)
				arguments["dgd_alpha"] = str(dalpha)
				arguments["dgd_gamma"] = str(dgamma)
				arguments["dgd_m"] = str(dgm)
		                prefix_str=orig_prefix
                		prefix_str=prefix_str+"_"+arguments["price_update_time"]+"_"+arguments["dgd_alpha"]+"_"+arguments["dgd_gamma"]+"_"+arguments["dgd_m"]
                		final_args=""
           			for arg_key in arguments:
                    			final_args = final_args+" --"+arg_key+"=\""+arguments[arg_key]+"\""
                #		cmd_line="nohup ./waf --run \""+sys.argv[1]+final_args+"\""+" > "+prefix_str+".out "+" 2> "+prefix_str+".err &"
#               cmd_line="python plot_qr.py "+prefix_str+"&"
                #cmd_line="python plot_onlyrates.py "+prefix_str+"&"
#                		cmd_line = "python find_multiple_events.py "+prefix_str+".out mp 10 >"+prefix_str+"_ct &"
        #        		cmd_line="nohup ./waf --run \""+sys.argv[1]+final_args+"\""+" > "+prefix_str+".out "+" 2> "+prefix_str+".err &"
#               cmd_line="python plot_qr.py "+prefix_str+"&"
                #cmd_line="python plot_onlyrates.py "+prefix_str+"&"
                		cmd_line1 = "python find_multiple_events.py "+prefix_str+".out mp 100  >"+prefix_str+"_ct "
			        cmd_line2 = "grep 'maximum' "+prefix_str+"_ct | cut -d ' ' -f 2 > " + prefix_str+"_cdf" 
                		cmd_line3 = "python plot_cdf.py "+prefix_str +"&"
				cmd_line = cmd_line1+"&&"+cmd_line2+"&&"+cmd_line3
                #cmd_line="python plot_onlyrates_all.py "+prefix_str+"&"
				print(cmd_line)
               			subprocess.call(cmd_line, shell="True")
f.close()

