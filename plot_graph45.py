import sys
import os
import matplotlib;
matplotlib.use('Agg');
del matplotlib;
import matplotlib.pyplot as plt;

min_file = sys.argv[1];
max_file = sys.argv[2];
min_plot_name = sys.argv[3];
max_plot_name = sys.argv[4];

alphas = []
min_rate = []
max_rate = []

# how data looks:
# max_throughput  0.1 42979.5 54329.1 50098.2266667   2263.34489211

with open(min_file, 'r') as f:
    for line in f:
      l1 = line.strip();
      xy = l1.split();

      avg_col = 4

      rate = float(xy[avg_col])/1000.0  
      min_rate.append(rate)
      print(xy, rate)

    f.close()

with open(max_file, 'r') as f:
    for line in f:
      l1 = line.strip();
      xy = l1.split();

      alpha_val = float(xy[1])  
      alphas.append(alpha_val)
      print(xy,alpha_val)

      avg_col = 4

      rate = float(xy[avg_col])/1000.0  
      max_rate.append(rate)
      print(xy, rate)

    f.close()


print(alphas, max_rate, min_rate)

plt.figure(1)
plt.plot(alphas, max_rate, '-bo')
plt.xlabel('Alpha')
plt.ylabel('Total rate (Gbps)')
plt.legend(loc='lower right')
plt.savefig(max_plot_name)

plt.figure(2)
plt.plot(alphas, min_rate, '-bo')
plt.xlabel('Alpha')
plt.ylabel('Minimum rate (Gbps)')
plt.legend(loc='lower right')
plt.savefig(min_plot_name)
