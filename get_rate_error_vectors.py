#!/usr/bin/python

import sys
import os

ideal_rates = {}
xf_rates = {}
findex=1
fstartindex=3
fsizeindex=5


error_correction = 0.016

f = open(sys.argv[1],'r')
for line in f:
  elems = []
  elems = (line.rstrip()).split(' ')
  if(elems[0] == "removing"):
    fid = int(elems[6])
    time_taken = (float(elems[11]) + 0.016)/1000  # get it in seconds
    flow_size = int(elems[13])

    print("flow %d fct %f" %(fid,time_taken))
    ideal_rates[fid] = flow_size/(1000000000.0*time_taken) # GBps

fstarts = {}
fsizes = {}
fstops = {}
f1 = open(sys.argv[2], 'r')
for line in f1:
  elems = []
  elems = (line.rstrip()).split(' ')

  if(elems[0] == "flow_start"):
    fid = int(elems[findex])
    fstart =  float(elems[fstartindex])
    fsize = float(elems[fsizeindex])
  
    fstarts[fid] = fstart
    fsizes[fid] = fsize

  if(elems[0] == "flow_stop"):
    fid = int(elems[findex])
    fstop =  float(elems[fstartindex])
    fsize = float(elems[fsizeindex])

    fstops[fid] = fstop

error = {}

for key in fstarts:
  if(key in fstops):
    ftime = (fstops[key] - fstarts[key])
    #print("Flow %d Size %d bytes finished in %f normalized %f" %(key, fsizes[key], ftime, ftime/fsizes[key]))
    xf_rates[key] = fsizes[key]/ftime

    #... and compare..
    error[key] = (xf_rates[key]-ideal_rates[key])/ideal_rates[key]
    print("ERROR fid %d error %f xf %f ideal %f" %(key,error[key],xf_rates[key],ideal_rates[key]))


  
  
