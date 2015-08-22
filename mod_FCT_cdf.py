#!/usr/bin/python
import matplotlib;
matplotlib.use('Agg');
del matplotlib;
import numpy as np;
import matplotlib.pyplot as plt;
import collections
import sys
import scipy
from scipy.stats import cumfreq

num_bins =  10000000
labels = []

def usage():
    print "input_file_list legend_list out_plot"

NUM_ARGS = 3

if(len(sys.argv[1:])!=NUM_ARGS):
    usage()
    exit()

input_files_f = sys.argv[1]
legend_list_f = sys.argv[2]
out_fname = sys.argv[3]


def cyret1(b):
  a = np.asarray(b)
  counts, bin_edges = np.histogram(a, bins=num_bins, normed=True)
#  counts, bin_edges = np.histogram(a, bins=num_bins, density=True)
  cdf = np.cumsum(counts)
  cdf = np.cumsum(counts*np.diff(bin_edges))
  #cdf /= cdf[len(cdf)-1]
  return cdf

def list_from_textfile(in_fname):
  d = []
  with open(in_fname,'r') as f:
      for line in f:
         print line
         v = line.split('\n')[0].split('\t')[0]
         print v
         d.append(v)
  return d


f = []

labels = list_from_textfile(legend_list_f)
filelist = list_from_textfile(input_files_f)


print labels, filelist

for f_SC in filelist:
    f.append(open(f_SC))

#for i in range (1, len(sys.argv)-1):
#  print ("opening file %s" %sys.argv[i])
#  l1=sys.argv[i].split('_')
#  labels.append(l1[2])
#  f.append(open(sys.argv[i]))

xy = []
x = []
listoflists = []

#for i in range(0, (len(sys.argv)-2)):
for i in range(0, len(filelist)):
  x = []
  for line in f[i]:
    l1 = line.rstrip();
    xy = l1.split(' ');
    x.append(float(xy[0]));
  listoflists.append(x)


plt.figure(1)
plt.title("CDF of raw FCTs")

#for i in range(0, (len(sys.argv)-2)):
for i in range(0, len(filelist)):
   y = cyret1(listoflists[i])
   #X1  = np.linspace(min(listoflists[i]),max(listoflists[i]),num_bins)
   X1  = np.linspace(min(listoflists[i]),max(listoflists[i]),num_bins)
#   print (X1)
#   print (y)
#   plt.xlim(0,0.01)
   plt.plot(X1, y, label=labels[i])
   plt.legend(loc='lower right')
   plt.xlabel('Time in milliseconds')
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

#plt.show()
#plt.savefig('%s.jpg' %(sys.argv[len(sys.argv)-1]))

plt.savefig(out_fname)

