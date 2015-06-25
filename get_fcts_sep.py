#!/usr/bin/python

#calculate finishing times of the flows

import os
import sys
import numpy as np
logs = open(sys.argv[1], "r")

fstarts = {}
fstops = {}
fsizes={}
fknowns = {}

findex=1
fstartindex=3
fsizeindex=5
fknownindex=10


for line in logs:
  l1 = line.rstrip()
  elems = l1.split(' ')
  if(elems[0] == "flow_start"):
    fid = int(elems[findex])
    fstart =  float(elems[fstartindex])
    fsize = float(elems[fsizeindex])
    fknown = int(elems[fknownindex])
  
    fstarts[fid] = fstart
    fsizes[fid] = fsize
    fknowns[fid] = fknown
    print("fid %d known %d" %(fid, fknowns[fid]))

  if(elems[0] == "flow_stop"):
    fid = int(elems[findex])
    fstop =  float(elems[fstartindex])
    fsize = float(elems[fsizeindex])

    fstops[fid] = fstop


for key in fstarts:
  if(key in fstops):
    ftime = (fstops[key] - fstarts[key])/1000000.0
    if(fknowns[fid] == 1):
      print("KnownFlow %d Size %d bytes finished in %f normalized %f" %(key, fsizes[key], ftime, ftime/fsizes[key]))
    else:
      print("UnknownFlow %d Size %d bytes finished in %f normalized %f" %(key, fsizes[key], ftime, ftime/fsizes[key]))

  
    

