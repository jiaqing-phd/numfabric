import sys
import os
import matplotlib;
matplotlib.use('Agg');
del matplotlib;
import matplotlib.pyplot as plt;

f_1 = sys.argv[1];
out_file = sys.argv[2];

alphas = []
min_rate = []

f = open(f_1, 'r')

MIN_FLAG= False
AVG_FLAG= False

for line in f:
  l1 = line.strip();
  xy = l1.split();
  if(xy[0] == "alpha"):
    alpha_val = float(xy[1])  
    alphas.append(alpha_val)
    print(xy,alpha_val)

  if(xy[0] == "min"):
    rate = float(xy[1])/1000.0  
    min_rate.append(rate)
    print(xy, rate)
    
    MIN_FLAG = True

  if(xy[0] == "average"):
    rate = float(xy[1])/1000.0  
    min_rate.append(rate)
    print(xy, rate)

    AVG_FLAG = True

f.close()

if(MIN_FLAG == True):
    plt.figure(1)
#    plt.title("Minimum Rate")
    plt.plot(alphas, min_rate, '-bo')
    plt.xlabel('Alpha')
    plt.ylabel('Minimum Rate (Gbps)')
    plt.legend(loc='lower right')
    plt.savefig(out_file)


if(AVG_FLAG == True):
    plt.figure(2)
#    plt.title("Maximum Rate")
    plt.plot(alphas, min_rate, '-bo')
    plt.xlabel('Alpha')
    plt.ylabel('Total rate (Gbps)')
    plt.legend(loc='lower right')
    plt.savefig(out_file)
