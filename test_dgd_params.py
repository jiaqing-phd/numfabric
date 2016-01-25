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
<<<<<<< HEAD
for pupdate_time in (0.00016, 0.00032):
=======
for pupdate_time in (0.0001, 0.00006):
>>>>>>> be7acbdf9d6b9c90ab6365e6331acddda0595f98
	fig_series=-1
	fig_series+=1
	for dalpha in (0.2, 0.3):
		for dgamma in (1.0, 10.0):
			for dgm in (0.0000000001, 0.000000001):
			        if (dgm == 0.000000001):
				   continue   
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
<<<<<<< HEAD
                		#cmd_line1="nohup ./waf --run \""+sys.argv[1]+final_args+"\""+" > "+prefix_str+".out "+" 2> "+prefix_str+".err "
#               cmd_line="python plot_qr.py "+prefix_str+"&"
                #cmd_line="python plot_onlyrates.py "+prefix_str+"&"
                		cmd_line2 = "python find_multiple_events.py "+prefix_str+".out mp 100 >"+prefix_str+"_ct &"
                		j=(j+1)%len(colors)
                		if (sim_only==1):
                		    #cmd_line2 = "grep 'maximum' "+prefix_str+"_ct | cut -f2 -d" " > "+prefix_str+"_cdf&"
				    #cmd_line=cmd_line1 +"&&"+ cmd_line2  
=======
                		cmd_line="nohup ./waf --run \""+sys.argv[1]+final_args+"\""+" > "+prefix_str+".out "+" 2> "+prefix_str+".err &"
                		j=(j+1)%len(colors)
                		if (sim_only==1):
				    cmd_line=cmd_line 
>>>>>>> be7acbdf9d6b9c90ab6365e6331acddda0595f98
				    #cmd_line= cmd_line1  
				    print(cmd_line2)
                		    subprocess.call(cmd_line2, shell="True")
                		
                		if (plot_only == 1):
                		    plot_right_metric.main(prefix_str, orig_prefix, fig_series, colors[j])
                		    cmd_line = "grep 'maximum' " + prefix_str + \
					            "_ct | cut -d ' ' -f 2 > " + prefix_str + "_cdf"
                		    #cmd_line = "grep 'converge_times' "+prefix_str+"_ct | cut -d ' ' -f 2 > " + prefix_str+"_cdf" 
        #        		cmd_line="nohup ./waf --run \""+sys.argv[1]+final_args+"\""+" > "+prefix_str+".out "+" 2> "+prefix_str+".err &"
#               cmd_line="python plot_qr.py "+prefix_str+"&"
                #cmd_line="python plot_onlyrates.py "+prefix_str+"&"
                		cmd_line1 = "python find_multiple_events.py "+prefix_str+".out mp 100  >"+prefix_str+"_ct "
			        cmd_line2 = "grep 'maximum' "+prefix_str+"_ct | cut -d ' ' -f 2 > " + prefix_str+"_cdf" 
                		cmd_line3 = "python plot_cdf.py "+prefix_str +"&"
				cmd_line = cmd_line1+"&&"+cmd_line2+"&&"+cmd_line3
                		#cmd_line1 = "python find_multiple_events.py "+prefix_str+".out mp 10 >"+prefix_str+"_ct "
			        #cmd_line2 = "grep 'maximum' "+prefix_str+"_ct | cut -d ' ' -f 2 > " + prefix_str+"_cdf" 
                		#cmd_line3 = "python plot_cdf.py "+prefix_str +"&"
				#cmd_line = cmd_line1+"&&"+cmd_line2+"&&"+cmd_line3
                #cmd_line="python plot_onlyrates_all.py "+prefix_str+"&"
				#print(cmd_line)
               			#subprocess.call(cmd_line, shell="True")
f.close()

