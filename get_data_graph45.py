import sys
import os
import matplotlib;
matplotlib.use('Agg');
del matplotlib;
import matplotlib.pyplot as plt;
import numpy as np;

f = open(sys.argv[1])
pre = sys.argv[1];
alpha = sys.argv[2]
out_file_min = sys.argv[3]
out_file_max = sys.argv[4]

DestRates = {}
TotalRates = {}

for line in f:
  l1 = line.rstrip();
  xy = l1.split(' ');

  if(xy[0] == "DestRate"):
    flow_id=int(xy[2])
    time = xy[3]
    rate = float(xy[4])

    if(time not in DestRates):
      DestRates[time] = []
    DestRates[time].append(rate)

  if(xy[1] == "TotalRate"):
    time = xy[0]
    rate = float(xy[2])

    if(time not in TotalRates):
      TotalRates[time] = []
    TotalRates[time].append(rate)

# process minimum_rates
#################################################
min_vector = []
for time, rate_vector in DestRates.iteritems():
    
    if(float(time) > 1.15):
        min = np.min(rate_vector)
        min_vector.append(min)

# write the min statistics
min_min = np.min(min_vector)
max_min = np.max(min_vector)
mean_min = np.mean(min_vector)
std_min = np.std(min_vector)

print('min', min_min, max_min, mean_min, std_min)

with open(out_file_min, 'a') as f:
    out_str = "\t".join(['min_throughput', str(alpha), str(min_min), str(max_min), str(mean_min), str(std_min)])
    f.write(out_str + "\n")

# process maximum_rates
#################################################
min_vector = []
for time, rate_vector in TotalRates.iteritems():
    
    if(float(time) > 1.15):
        min = np.min(rate_vector)
        min_vector.append(min)

# write the min statistics
min_min = np.min(min_vector)
max_min = np.max(min_vector)
mean_min = np.mean(min_vector)
std_min = np.std(min_vector)

print('max', min_min, max_min, mean_min, std_min)

with open(out_file_max, 'a') as f:
    out_str = "\t".join(['max_throughput', str(alpha), str(min_min), str(max_min), str(mean_min), str(std_min)])
    f.write(out_str + "\n")


