import sys
import os
import matplotlib;
matplotlib.use('Agg');
del matplotlib;
import matplotlib.pyplot as plt;
import numpy as np;

FCT_file = sys.argv[1];
FCT_CDF_plot_out = sys.argv[2];

# how FCT file will look:
# FLOW_ID FCT_XFABRIC FCT_DGD FCT_CENTRALIZED

num_bins = 1000 
error_xfabric_vector = []
error_dgd_vector = []

## THIS IS INCOMPLETE CURRENTLY SINCE KANTHI SEEMS TO HAVE THIS PLOT
#################################################################

def get_cdf(b):
  a = np.asarray(b)
  counts, bin_edges = np.histogram(a, bins=num_bins, normed=True)
  cdf = np.cumsum(counts)
  cdf = np.cumsum(counts*np.diff(bin_edges))
  #cdf /= cdf[len(cdf)-1]
  return cdf

with open(FCT_file, 'r') as f:
    for line in f:
        a = line.split()

        flow_id = a[0]
        FCT_xfabric = float(a[1])
        FCT_dgd = float(a[2])
        FCT_centralized = float(a[3])

        error_xfabric = abs(FCT_xfabric - FCT_centralized)/(FCT_centralized) 
        error_dgd = abs(FCT_dgd - FCT_centralized)/(FCT_centralized) 

        error_xfabric_vector.append(error_xfabric)
        error_dgd_vector.append(error_dgd)

# plot the CDFs for the various quantities

print(error_dgd_vector, error_xfabric_vector)

# DGD errors
x_dgd  = np.linspace(min(error_dgd_vector),max(error_dgd_vector),num_bins)
y_dgd = get_cdf(x_dgd) 

x_xfabric  = np.linspace(min(error_xfabric_vector),max(error_xfabric_vector),num_bins)
y_xfabric = get_cdf(x_xfabric) 

labels = ['DGD error', 'xfabric error']


print(labels, x_dgd, x_xfabric, y_dgd, y_xfabric)

plt.figure(1)
plt.plot(x_dgd, y_dgd, label=labels[0])
plt.legend(loc='lower right')

plt.plot(x_xfabric, y_xfabric, label=labels[1])
plt.legend(loc='lower right')

plt.title("CDF of Distributed vs Centralized Allocations")
plt.xlabel('FCT error')
plt.ylabel('Probability')

plt.savefig(FCT_CDF_plot_out)
