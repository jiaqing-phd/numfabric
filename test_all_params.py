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
#for pupdate_time in (0.000016, 0.000032,0.000048, 0.000064):
#  for gupdate_time in (0.0, 0.000016, 0.000032):
#for pupdate_time in (0.0001, 0.00015, 0.0002):
for pupdate_time in (0.0001, 0.000064, 0.0002):
  for gupdate_time in (0.000032, 0.0):
    if(float(pupdate_time) <= float(gupdate_time)):
        continue
    for ewma_time in (10000, 20000):
        if(ewma_time == 20000):
            continue
        for dt_val in (0.000012, 0.000024):
            for eta_val in (1.0, 10.0):
                if(eta_val == 1.0):
                    continue
                arguments["price_update_time"] = str(pupdate_time)
                arguments["guardtime"] = str(gupdate_time)
                arguments["dt_val"] = str(dt_val)
                arguments["kvalue_rate"] = str(ewma_time)
                arguments["xfabric_eta"] = str(eta_val)
                prefix_str=orig_prefix
                prefix_str=prefix_str+"_"+arguments["price_update_time"]+"_"+arguments["guardtime"]+"_"+arguments["dt_val"]+"_"+arguments["kvalue_rate"]+"_"+arguments["xfabric_eta"]
                final_args=""
                for arg_key in arguments:
                    final_args = final_args+" --"+arg_key+"=\""+arguments[arg_key]+"\""
                cmd_line="nohup ./waf --run \""+sys.argv[1]+final_args+"\""+" > "+prefix_str+".out "+" 2> "+prefix_str+".err &"
#                cmd_line="python plot_qr.py "+prefix_str+"&"
                print(cmd_line)
                subprocess.call(cmd_line, shell="False")
f.close()

