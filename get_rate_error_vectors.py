#!/usr/bin/python

import sys
import os
import time
import subprocess

ideal_rates = {}
xf_rates = {}
start_times = {}
findex=1
fstartindex=3
fsizeindex=5

def small_flow(flow_size):
    if(flow_size<100000):
        return true
    return false

def medium_flow(flow_size):
    if(flow_size >= 100000 and flow_size <1000000):
        return true
    return false

#error_correction = 0.016 + 0.008  #SYN-SYNACK + 1/2 RTT PROPAGATION
error_correction = 0.016  +0.008441#SYN-SYNACK + 1/2 RTT PROPAGATION

#prefix=sys.argv[1]
#ns3_log=prefix+".out"
#ideal_fcts=prefix+".ideal"
ns3_log=sys.argv[1]
ideal_fcts=sys.argv[2]

fstarts = {}
fsizes = {}
fstops = {}
f1 = open(ns3_log, 'r')
for line in f1:
  elems = []
  elems = (line.rstrip()).split(' ')

  if(elems[0] == "flow_start" and len(elems) >6 ):
    fid = int(elems[findex])
    fstart =  float(elems[fstartindex])
    fsize = float(elems[fsizeindex])
  
    fstarts[fid] = fstart
    fsizes[fid] = fsize

  if(elems[0] == "flow_stop" and len(elems) > 6):
    fid = int(elems[findex])
    fstop =  float(elems[fstartindex])
    fsize = float(elems[fsizeindex])

    fstops[fid] = fstop

# get ideal fcts first
#ideal_rate_cmd = "python extract_graph_pfabric.py "+ns3_log+" mp > "+ideal_fcts
#proc = subprocess.Popen(ideal_rate_cmd, shell="False")
#proc.wait()
#time.sleep(5)

# make sure the file is written by sleeping 

f = open(ideal_fcts,'r')
for line in f:
  elems = []
  elems = (line.rstrip()).split(' ')
  if(elems[0] == "removing" and len(elems) > 14):
    #print elems
    fid = int(elems[6])
    #time_taken = (float(elems[11]) + error_correction)/1000  # get it in seconds
    time_taken = float(elems[11])/1000000.0   # get it in milli-seconds
    flow_size = int(elems[13])
    print("tt %f %f %f" %(time_taken, error_correction, (flow_size*1.0/1500.0)*0.018))
    if(flow_size <= 1500):
        time_taken = time_taken + error_correction+(flow_size*1.0/1500.0)*0.0018
    else:
        time_taken = time_taken + error_correction+0.0018

    ideal_rates[fid] = flow_size/(1000000.0*time_taken) # GBps
    #print("ideal rate %f time taken %f flow size %f" %(ideal_rates[fid],time_taken,flow_size))
    start_times[fid] = float(elems[15])
error = {}

#error_correction = 0.026313
for key in fstarts:
  if(key in fstops and key in ideal_rates):
    #ftime = (fstops[key] - fstarts[key]) - (error_correction*1000000.0) # all in ns
    ftime = (fstops[key] - fstarts[key]) 
#    diff = (fstops[key] - fstarts[key])
    #print("Flow %d Size %d bytes finished in %f normalized %f" %(key, fsizes[key], ftime, ftime/fsizes[key]))
    xf_rates[key] = fsizes[key]/ftime
    #print("Flow %d size %f start %f finish %f rate %f ftime %f diff %f correction %f" %(key,fsizes[key],fstarts[key],fstops[key],xf_rates[key], ftime, diff, error_correction*1000000.0 ))
    #... and compare..
    error[key] = (xf_rates[key]-ideal_rates[key])/ideal_rates[key]
    #... where to write?
    print("ERROR fid %d error %f xf %f ideal %f flowsize %f starttime %f" %(key,error[key],xf_rates[key],ideal_rates[key],fsizes[key], start_times[key]))
#  else:
#    print("Flow %d started at %f size %f did not stop" %(key, fsizes[key], fstarts[key]))
  
