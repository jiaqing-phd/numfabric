#!/usr/bin/python
import sys
import os
import subprocess
from subprocess import Popen, PIPE
import matplotlib.pyplot as plt
import plot_cdf_func
import plot_right_metric

plt.close("all")

arguments = {}

if(len(sys.argv) < 2):
  print("Usage : python run_config.py <executable> <config_file> <plot or not>")
  sys.exit()

colors = ['r','b','g', 'm', 'c', 'y','k','#fedcba','#abcdef' ]#\
#,'#ababab','#badaff','#deadbe','#bedead','#afafaf','#8eba42','#e5e5e5','#6d904f']
       
plot_only = 1
plot_script = "plot_qr.py"

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

i=-1
# print(arguments)
#print(arguments)
orig_prefix=arguments["prefix"]
#for pupdate_time in (0.000016, 0.000032,0.000048, 0.000064):
#  for gupdate_time in (0.0, 0.000016, 0.000032):
#for pupdate_time in (0.0001, 0.00015, 0.0002):


j=0
for pupdate_time in (0.000080, 0.000064, 0.0001):
  #if (pupdate_time == 0.000080):
  #   continue    
  for gupdate_time in (0.000025, 0.000016, 0.0):
    if(float(pupdate_time) <= float(gupdate_time)):
        continue
    if(gupdate_time != 0.0):
        continue   
    i+=1
    itime=1
    for rtime in (20000, 40000, 60000, 80000):
        if (rtime != 80000):# or rtime== 60000):  
            continue
        ptime = rtime

        for dt_val in (0.000012, 0.000024):
            if (dt_val == 0.000024):
                continue
            for eta_val in (1.0, 10.0):
                if(eta_val == 1.0):
                    continue
                j=(j+1)%len(colors)
                arguments["price_update_time"] = str(pupdate_time)
                arguments["guardtime"] = str(gupdate_time)
                arguments["dt_val"] = str(dt_val)
                arguments["kvalue_rate"] = str(rtime)
                arguments["kvalue_price"] = str(ptime)
                arguments["xfabric_eta"] = str(eta_val)
                prefix_str=orig_prefix
                prefix_str=prefix_str+"_"+arguments["price_update_time"]+"_"+arguments["guardtime"]+"_"+arguments["dt_val"]+"_"+arguments["kvalue_price"]+"_"+arguments["xfabric_eta"]
                final_args=""
                for arg_key in arguments:
                    final_args = final_args+" --"+arg_key+"=\""+arguments[arg_key]+"\""
                cmd_line="nohup ./waf --run \""+sys.argv[1]+final_args+"\""+" > "+prefix_str+".out "+" 2> "+prefix_str+".err &"
#               cmd_line="python plot_qr.py "+prefix_str+"&"
                #cmd_line="python plot_onlyrates.py "+prefix_str+"&"
                #cmd_line = "python find_multiple_events.py "+prefix_str+".out mp 10 >"+prefix_str+"_ct &"
                #cmd_line="python plot_onlyrates_all.py "+prefix_str+"&"
                #cmd_line = "python find_multiple_events_new.py "+prefix_str+".out mp >"+prefix_str+"_ct &"
                #cmd_line = "grep 'maximum' "+prefix_str+"_ct > out "
		#print(cmd_line)
                #subprocess.call(cmd_line, shell="True")
                if (plot_only == 0):
                    cmd_line = "python find_multiple_events.py " + \
                	    prefix_str + ".out mp 10 >" + prefix_str + "_ct &"
                    print(cmd_line)
                    subprocess.call(cmd_line, shell="False")
                if (plot_only == 1):
                    plot_right_metric.main(prefix_str, orig_prefix, itime, colors[j])
                    cmd_line = "grep 'maximum' " + prefix_str + \
			            "_ct | cut -d ' ' -f 2 > " + prefix_str + "_cdf"
                    #cmd_line = "grep 'converge_times' "+prefix_str+"_ct | cut -d ' ' -f 2 > " + prefix_str+"_cdf" 
                    #print(cmd_line)
                    #subprocess.call(cmd_line, shell="False")
                    #plot_cdf_func.main(prefix_str, orig_prefix, i, colors[j] )
                
j=(j+1)%len(colors)
prefix_str="csdgd_0.0001_0.3_10.0_1e-09"
#cmd_line = "python find_multiple_events.py " + \
#prefix_str + ".out mp 10 >" + prefix_str + "_ct "
#subprocess.call(cmd_line, shell="True")
#print(cmd_line)
plot_right_metric.main(prefix_str, orig_prefix, itime, colors[j])

               
plt.draw()
plt.show()
f.close()

