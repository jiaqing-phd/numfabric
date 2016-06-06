#!/usr/bin/python
import sys
import os
import subprocess
from subprocess import Popen, PIPE
import plot_right_metric

arguments = {}

if(len(sys.argv) < 2):
  print("Usage : python run_config.py <executable> <config_file> <plot or not>")
  sys.exit()
sim_only=1
plot_only=0
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

colors = ['r','b','g', 'm', 'c', 'y','k','#fedcba','#abcdef' ]#\

j=0
#print(arguments)
orig_prefix=arguments["prefix"]
#for pupdate_time in (0.000032, 0.0001):
#for pupdate_time in (0.000016,0.000032,0.000048):
for pupdate_time in (0.000016, 0.000032):
        if(pupdate_time == 0.000032):
            continue
	#for dalpha in (0.3):
        dalpha = 0.3
        dgamma = 10.0
        for dgm in (0.0000000001, 0.000000001, 0.00000000001):
	    arguments["price_update_time"] = str(pupdate_time)
	    arguments["dgd_alpha"] = str(dalpha)
	    arguments["dgd_gamma"] = str(dgamma)
	    arguments["dgd_m"] = str(dgm)
            prefix_str=orig_prefix
	    prefix_str=prefix_str+"_"+arguments["price_update_time"]+"_"+arguments["dgd_alpha"]+"_"+arguments["dgd_gamma"]+"_"+arguments["dgd_m"]
	    final_args=""
	    for arg_key in arguments:
	        final_args = final_args+" --"+arg_key+"=\""+arguments[arg_key]+"\""
	    cmd_line="nohup ./waf --run \""+sys.argv[1]+final_args+"\""+" > "+prefix_str+".out "+" 2> "+prefix_str+".err &"
            print (cmd_line)
	    subprocess.call(cmd_line, shell="True")

	
f.close()

