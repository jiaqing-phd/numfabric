#!/usr/bin/python

#calculate finishing times of the flows

import os
import sys
import numpy as np



def get_fcts(fname):
  fct = {}
  fstarts = {}
  fstops = {}
  fsizes={}
  fknowns = {}

  findex=1
  fstartindex=3
  fsizeindex=5
  #fknownindex=10

  # open the file
  logs = open(fname, "r")
  for line in logs:
    l1 = line.rstrip()
    elems = l1.split(' ')
    if(elems[0] == "flow_start"):
      fid = int(elems[findex])
      fstart =  float(elems[fstartindex])
      fsize = float(elems[fsizeindex])
      #fknown = int(elems[fknownindex])
  
      fstarts[fid] = fstart
      fsizes[fid] = fsize
      #fknowns[fid] = fknown
      #print("fid %d known %d" %(fid, fknowns[fid]))

    if(elems[0] == "flow_stop"):
      fid = int(elems[findex])
      fstop =  float(elems[fstartindex])
      fsize = float(elems[fsizeindex])

      fstops[fid] = fstop


  for key in fstarts:
    if(key in fstops):
      ftime = (fstops[key] - fstarts[key])/1000000.0
      #if(fknowns[key] == 1):
      #  print("KnownFlow %d Size %d bytes finished in %f normalized %f" %(key, fsizes[key], ftime, ftime/fsizes[key]))
      #else:
      #  print("UnknownFlow %d Size %d bytes finished in %f normalized %f" %(key, fsizes[key], ftime, ftime/fsizes[key]))
      fct[key] = ftime
    else:
      print("flow %d started at %f and did not stop" %(key, fstarts[key]))
  return fct 


fcts = get_fcts(sys.argv[1])
#ideal = get_fcts(sys.argv[2])

fcts_array = []
for key in fcts:
  fcts_array.append(fcts[key])
print("%f "%np.mean(fcts_array))

#  print("%f %f" %(fcts[key], ideal[key]))
#  if(key in ideal):
#    stretch = fcts[key]/ideal[key]
#    print("key %d stretch %f" %(key, stretch))
