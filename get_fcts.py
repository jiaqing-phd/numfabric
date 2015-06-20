#!/usr/bin/python

#calculate finishing times of the flows

import os
import sys
import numpy as np
logs = open(sys.argv[1], "r")

fstarts = {}
fstops = {}
fsizes={}

findex=1
fstartindex=3
fsizeindex=5

for line in logs:
  l1 = line.rstrip()
  elems = l1.split(' ')
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


for key in fstarts:
  if(key in fstops):
    ftime = (fstops[key] - fstarts[key])/1000000.0
    print("Flow %d Size %d bytes finished in %f normalized %f" %(key, fsizes[key], ftime, ftime/fsizes[key]))
  
    

