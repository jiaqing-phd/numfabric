#!/usr/bin/python

#calculate finishing times of the flows

import os
import sys
import numpy as np
import matplotlib.pyplot as plt;


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
      print("flow %d started at %f and did not stop %s flow_size %f" %(key, fstarts[key], fname, fsizes[key]))
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
    return "xfabric_util2_test"
  if(method == "strawman"):
    return "fct_strawman1"

def get_all_stretches(method):
  stretch = {}
  file_prefix = get_fileprefix(method)
  #for load in (0.2, 0.4, 0.6, 0.8):
  for load in (0.2, 0.4, 0.6):
    fname=file_prefix+"_"+str(load)+".out"
    base="fct_pfabric_"+str(load)+".out"
    print(len(get_stretch(fname, base)))
    stretch[load]=np.mean(get_stretch(fname, base))

  return stretch

stretch_str = {}
stretch_str["dctcp"] = get_all_stretches("dctcp")
stretch_str["xfabric"] = get_all_stretches("xfabric")
stretch_str["strawman"] = get_all_stretches("strawman")
print(stretch_str["dctcp"])
print(stretch_str["strawman"])
print(stretch_str["xfabric"])
i=0
markers=["s","d","p","v","+","*"]
for key in stretch_str:
  series = stretch_str[key]
  keys = []
  values = []
  for key2 in series:
    keys.append(key2)
    values.append(series[key2])
  print keys
  print values
  plt.plot(keys, values, label=str(key), linewidth=2, marker=markers[i], markersize=10)
  i = i+1
  
plt.legend(loc='upper right')
plt.xlabel("Load")
plt.ylabel("Average FCT (w.r.t. pFabric)")
plt.savefig("average_fcts.png")
plt.draw()
plt.show()
    




#print("XFABRIC")
#for key in stretch_str:
#  print("%s %f" %(key,stretch_str["xfabric"]))
     
