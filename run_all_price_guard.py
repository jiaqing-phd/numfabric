#!/usr/bin/python
import sys
import os
import subprocess
from subprocess import Popen, PIPE

arguments = {}

if(len(sys.argv) < 2):
  print("Usage : python run_config.py <executable> <config_file>")
  sys.exit()
plot_script=""
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

orig_prefix=arguments["prefix"]
for pupdate_time in (0.000016, 0.000020, 0.000040, 0.000060, 0.000080, 0.0001):
  for gupdate_time in (0.0, 0.000001, 0.000008, 0.00001,0.00002, 0.00004):
    for kval in (10000, 20000, 30000, 40000, 50000, 60000):
      arguments["price_update_time"] = str(pupdate_time)
      arguments["guardtime"] = str(gupdate_time)
      arguments["kvalue_rate"] = str(kval)
      arguments["kvalue_price"] = str(kval)
      prefix_str=orig_prefix
      prefix_str=prefix_str+"_"+arguments["price_update_time"]+"_"+arguments["guardtime"]+"_"+arguments["kvalue_price"]
      arguments["prefix"] = prefix_str

      final_args=""
      for arg_key in arguments:
        final_args = final_args+" --"+arg_key+"=\""+arguments[arg_key]+"\""

      cmd_line="nohup ./waf --run \""+sys.argv[1]+final_args+"\""+" > "+prefix_str+".out "+" 2> "+prefix_str+".err &"
      print(cmd_line)
      subprocess.call(cmd_line, shell="False")

#cmd_line="python "+plot_script+" "+prefix_str
#print(cmd_line)
#subprocess.call(cmd_line, shell="False")
f.close()

