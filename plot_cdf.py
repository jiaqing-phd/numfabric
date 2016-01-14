import matplotlib.pyplot as plt
import numpy as np

import sys
import os

# Choose how many bins you want here
num_bins = 20

filename = sys.argv[1]+"_cdf"
pre = sys.argv[1];
dir = sys.argv[1]

if not os.path.exists(dir):
  os.makedirs(dir)

#f=open('out', 'r')
#list=[]
#for line in f.readlines():
#	list.append(float(line))

data = np.loadtxt(filename)

# Use the histogram function to bin the data
#counts, bin_edges = np.histogram(data, bins=num_bins, normed=True)

# Now find the cdf
#cdf = np.cumsum(counts)

colors = ['r','b','g', 'm', 'c', 'y','k']
i=1;

plt.figure(1)
plt.title(pre)
# And finally plot the cdf
#plt.plot(bin_edges[1:], cdf, colors[i], label=pre)

sorted_data = np.sort(data)

yvals=np.arange(len(sorted_data))/float(len(sorted_data))

plt.plot(sorted_data,yvals)

plt.xlabel('Convergence Time in seconds')
plt.ylabel('CDF ')
plt.legend(loc='best')
plt.savefig('%s/%s.%s.jpg' %(pre,pre,"cdf"))

#plt.draw()
#plt.show()

#
#gaussian_numbers = np.random.randn(1000)
#plt.hist(gaussian_numbers)
#plt.title("Gaussian Histogram")
#plt.xlabel("Value")
#plt.ylabel("Frequency")
#plt.draw()
#plt.show()
