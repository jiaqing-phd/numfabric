#!/usr/bin/python

import matplotlib.pyplot as plt
import matplotlib as mp
import sys
import scipy
from scipy.stats import cumfreq
import numpy as np
import statsmodels.api as sm # recommended import according to the docs
import seaborn as sns

plt.gcf().subplots_adjust(bottom=0.15)

#plt.gca().tight_layout()

mp.rcParams.update({"font.size":22})
labels = []
num_bins = 100
def cyret1(b):
  a = np.asarray(b)
  #print(a)
  #num_bins =  len(b)/2.0
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
  #labels.append(sys.argv[i])
  if(i == 1):
   labels.append("NUMFabric")
  if(i == 2):
    labels.append("DGD")
  if(i == 3):
    labels.append("RCP")
  f.append(open(sys.argv[i]))

xy = []
x = []
listoflists = []

for i in range(0, (len(sys.argv)-2)):
  x = []
  for line in f[i]:
    l1 = line.rstrip();
    xy = l1.split(' ');
    val = float(xy[0])*1000.0
    #print ("%f" %val)
    if(val >= 0.0 and val < 100):
        x.append(val);
    #    print ("appending %f" %val)
    #else:
    #    print ("discarding 0")
#    x.append(float(l1));
  listoflists.append(x)


#plt.figure(num=None, figsize=(16,9))
plt.figure(1)
#plt.title("CDF of convergence times")
colors=('r','b','g','c','m','k','y')
i=0
for i in range(0, (len(sys.argv)-2)):
#   y = cyret1(listoflists[i])
   ecdf = sm.distributions.ECDF(listoflists[i])
   X1  = np.linspace(min(listoflists[i]),max(listoflists[i]))
   y1 = ecdf(X1)
#   print (len(X1))
#   print (len(y))
   #plt.xlim(0,10000000.0)
   #plt.semilogx(X1, y, label=labels[i], linewidth=2)
   sns.set_style("darkgrid")
   sns.set_context("notebook", font_scale=2.0, rc={"lines.linewidth": 2.5})
   plt.plot(X1, y1, label=labels[i], linewidth=2, color=colors[i])
   i = (i + 1)%len(colors)
   plt.xlabel('Time (ms)')
   plt.xlim(0,2)
   #plt.xlabel('Time in seconds')
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
plt.legend(loc="lower right")
print("Drawing the plot.... ")
plt.draw()
plt.savefig('%s.pdf' %(sys.argv[len(sys.argv)-1]))
plt.show()
