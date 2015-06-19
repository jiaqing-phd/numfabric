#!/usr/bin/python

#calculate finishing times of the flows

import os
import sys
import numpy as np
logs = open(sys.argv[1], "r")
out_file='FCT.' + sys.argv[1]
verbose_out_file='verbose.FCT.' + sys.argv[1]

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


f_verbose = open(verbose_out_file, 'w')

# out file, very small one
with open(out_file, 'w') as f:
  for key in fstarts:
    if(key in fstops):
      ftime = (fstops[key] - fstarts[key])/1000000.0
      print("Flow %d Size %d bytes finished in %fms" %(key, fsizes[key], ftime))
     
      x = "Flow %d Size %d bytes finished in %fms" %(key, fsizes[key],
      ftime) + '\n'
      
      f_verbose.write(x)

      # x/s
      time_normalized = float(ftime)/float(fsizes[key])
      
      # ratio = time/minimum_time to finish
      
      out_line = '\t'.join([str(key), str(fsizes[key]), str(ftime),
      str(time_normalized)])
      f.write(out_line + '\n')

f_verbose.close()    

