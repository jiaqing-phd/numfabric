#!/usr/bin/python

import os
import sys

def get_flow_data(fname):
  recvd = {}
  f = open(fname, "r")
  for line in f:
    l1 = line.rstrip()
    xy = l1.split(' ')
    if(xy[0] == "TotalRecvd"):
      fid = int(xy[3])
      if(int(xy[4]) > 0 and (fid == 1 or fid == 2 or fid == 3 or fid == 4)):
        recvd[fid] = xy[4]

  return recvd

ret = get_flow_data(sys.argv[1])

for key in ret:
  print("%s %s" %(key, ret[key]))
