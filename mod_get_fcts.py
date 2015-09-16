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

FCT={}
FCT_normalized={}

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
      #print("Flow %d Size %d bytes finished in %fms" %(key, fsizes[key], ftime))
     
      x = "Flow %d Size %d bytes finished in %fms" %(key, fsizes[key],
      ftime) + '\n'
      
      f_verbose.write(x)

      # x/s
      time_normalized = float(ftime)/float(fsizes[key])
      
      # ratio = time/minimum_time to finish
     
      FCT[key] = ftime
      FCT_normalized[key] = time_normalized
      
      out_line = '\t'.join([str(key), str(fsizes[key]), str(ftime),
      str(time_normalized)])
      f.write(out_line + '\n')

f_verbose.close()    

## aggregate stats on fsizes
myarray = fsizes.values()

n_counts,bin_edges = np.histogram(myarray,bins=11,normed=True) 
cdf = np.cumsum(n_counts)  # cdf not normalized, despite above
scale = 1.0/cdf[-1]
ncdf = scale * cdf

#print(n_counts, cdf, scale, ncdf)

smallest_50_cutoff = np.percentile(myarray, 50) 
largest_99_cutoff = np.percentile(myarray, 99)

print(smallest_50_cutoff, largest_99_cutoff)

# smallest 50
#################################
smallest_50_keys = []
smallest_50_FCTs = []
smallest_50_FCTs_norm = []
for k,v in fsizes.iteritems():
    if(v <= smallest_50_cutoff):
        smallest_50_keys.append(k)
        smallest_50_FCTs.append(FCT[k])
        smallest_50_FCTs_norm.append(FCT_normalized[k])

print('small FCT', np.mean(smallest_50_FCTs), np.mean(smallest_50_FCTs_norm), len(smallest_50_FCTs))

# largest 100
#################################
largest_99_keys = []
largest_99_FCTs = []
largest_99_FCTs_norm = []
for k,v in fsizes.iteritems():
    if(v >= largest_99_cutoff):
        largest_99_keys.append(k)
        largest_99_FCTs.append(FCT[k])
        largest_99_FCTs_norm.append(FCT_normalized[k])

print('large FCT', np.mean(largest_99_FCTs), np.mean(largest_99_FCTs_norm), len(largest_99_FCTs))


