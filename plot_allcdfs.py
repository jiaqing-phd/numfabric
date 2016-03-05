#!/usr/bin/python

import matplotlib.pyplot as plt
import matplotlib as mp
import sys
import scipy
from scipy.stats import cumfreq
import numpy as np

mp.rcParams.update({"font.size":22})
num_bins =  100
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

for i in range (1, len(sys.argv)-1):
  print ("opening file %s" %sys.argv[i])
  l1=sys.argv[i].split('_')
  labels.append(sys.argv[i])
  f.append(open(sys.argv[i]))

xy = []
x = []
listoflists = []

for i in range(0, (len(sys.argv)-2)):
  x = []
  for line in f[i]:
    l1 = line.rstrip();
    #xy = l1.split(' ');
    #x.append(float(xy[0]));
    x.append(float(l1));
  listoflists.append(x)


plt.figure(1)
plt.title("CDF of convergence times")

for i in range(0, (len(sys.argv)-2)):
   y = cyret1(listoflists[i])
   #X1  = np.linspace(min(listoflists[i]),max(listoflists[i]),num_bins)
   X1  = np.linspace(min(listoflists[i]),max(listoflists[i]),num_bins)
   #print (X1)
   #print (y)
   plt.xlim(0,0.1)
   plt.plot(X1, y, label=labels[i], linewidth=2)
   plt.xlabel('Time in seconds')
   plt.ylabel('Probability')

#  a = numpy.asarray(listoflists[i])
#  num_bins =  20
#  counts, bin_edges = numpy.histogram(a, bins=num_bins, normed=True)
#  cdf = numpy.cumsum(counts)
  #pylab.plot(bin_edges[1:], cdf)
  #b = cumfreq(a, num_bins)
#  plt.plot(bin_edges[1:], cdf)

#dummyx=list(range(0, len(x)))
#plt.plot(x, y, 'r') 
#plt.plot(dummyx, x, 'r') 
#plt.xlabel('Time in seconds')
#plt.ylabel('%s' %sys.argv[3])
#plt.savefig('%s.jpg' %sys.argv[2])

plt.show()
plt.savefig('%s.jpg' %(sys.argv[len(sys.argv)-1]))
