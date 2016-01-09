#!/usr/bin/python

import matplotlib.pyplot as plt
import sys
import scipy
from scipy.stats import cumfreq
import numpy as np

num_bins =  1000000
labels = []

def cyret1(b):
  a = np.asarray(b)
  #print(a)
  counts, bin_edges = np.histogram(a, bins=num_bins, normed=True)
#  counts, bin_edges = np.histogram(a, bins=num_bins, density=True)
  cdf = np.cumsum(counts)
  cdf = np.cumsum(counts*np.diff(bin_edges))
  #cdf /= cdf[len(cdf)-1]
  return cdf



f = []
sys_args = []

for i in range (1, (len(sys.argv)-3)):
  print ("opening file %s" %sys.argv[i])
  sys_args=sys.argv[i].split('_')
  print("label = %s" %(sys_args[int(sys.argv[len(sys.argv)-2])]))
  labels.append(sys_args[int(sys.argv[len(sys.argv)-2])])
  f.append(open(sys.argv[i]))

xy = []
x = []
listoflists = []

for i in range(0, (len(sys.argv)-4)):
  x = []
  for line in f[i]:
    l1 = line.rstrip();
    if(float(l1) < 0.01):
        x.append(float(l1));
  listoflists.append(x)


plt.figure(1)
print(sys_args)
title_str="Varying_"+sys.argv[len(sys.argv)-3]+"_Price_"+sys_args[1]+"_Guard_"+sys_args[2]+"_dt_"+sys_args[3]
plt.title(title_str)

for i in range(0, (len(sys.argv)-4)):
   y = cyret1(listoflists[i])
   X1  = np.linspace(min(listoflists[i]),max(listoflists[i]),num_bins)
   plt.plot(X1, y, label=labels[i])
   plt.legend(loc='lower right')
   plt.xlabel('Time in seconds')
   plt.ylabel('Probability')

plt.draw()
plt.savefig('%s.png' %(title_str))
plt.show()
