#!/usr/bin/python
import sys
import os
import subprocess
from subprocess import Popen, PIPE

arguments = {}

if(len(sys.argv) < 2):
  print("Usage : python run_config.py <executable> <config_file> <plot or not>")
  sys.exit()

plot_script="plot_rates.py"

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
for pupdate_time in ("1.6e-05", "3.2e-05","4.8e-05", "6.4e-05"):
  for gupdate_time in ("0.0", "1.6e-05", "3.2e-05"):
    if(pupdate_time <= gupdate_time):
        next
    ewma_time=10000
    for dt_val in ("1.2e-05", "2.4e-05"):
        prefix_str=orig_prefix
        prefix_str=prefix_str+"_"+pupdate_time+"_"+gupdate_time+"_"+dt_val+"_"+str(ewma_time)
        # Let's first plot convergence CDF
        parse2_str = "more "+prefix_str+"_converge_times_longer.out | grep \"converge_times: \" | cut -f2 -d\" \" > "+prefix_str+"_ctimes_only"
        print(parse2_str)
        subprocess.call(parse2_str, shell="False")
f.close()

