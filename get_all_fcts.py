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
      fct[key] = ftime
    else:
      print("flow %d started at %f and did not stop" %(key, fstarts[key]))
  return fct 


def get_stretch(file1, file2):
  fcts = get_fcts(file1)
  ideal = get_fcts(file2)
  stretch = []

  for key in fcts:
  #  print("%f %f" %(fcts[key], ideal[key]))
    if(key in ideal):
      stretch.append(fcts[key]/ideal[key])
  return stretch

def get_fileprefix(method):
  if(method=="dctcp"):
    return "fct_dctcp"
  if(method == "xfabric"):
    return "xfabric_util2"
  if(method == "strawman"):
    return "fct_strawman"

def get_all_stretches(method):
  stretch = {}
  file_prefix = get_fileprefix(method)
  #for load in (0.02, 0.04, 0.06, 0.08, 0.10):
  for load in (0.02, 0.04):
    fname=file_prefix+"_"+str(load)+".out"
    base="fct_pfabric_"+str(load)+".out"
    print(len(get_stretch(fname, base)))
    stretch[load]=np.mean(get_stretch(fname, base))

  return stretch

stretch_str = {}
stretch_str["dctcp"] = get_all_stretches("dctcp")
#stretch_str["xfabric"] = get_all_stretches("xfabric")
#stretch_str["strawman"] = get_all_stretches("strawman")
print(stretch_str["dctcp"])
#print(stretch_str["xfabric"])
#print(stretch_str["strawman"])





#print("XFABRIC")
#for key in stretch_str:
#  print("%s %f" %(key,stretch_str["xfabric"]))
     
