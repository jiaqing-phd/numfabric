#!/usr/bin/python

import matplotlib.pyplot as plt
import sys


f = open(sys.argv[1])
xy = []
x = {}
y = {}

for line in f:
  l1 = line.rstrip();
  xy = l1.split(' ');
  if(xy[0]=="rtt_rate"):
    qid = xy[2]
    if(qid not in x):
      x[qid] = []
      y[qid] = []

    x[qid].append(float(xy[1]))
    y[qid].append(float(xy[3]));

plt.figure(1)
plt.title("RTT BASED RATES")

for key in x:
  plt.plot(x[key], y[key], label=key) 
plt.legend(loc='upper right')
#plt.plot(xaxis, x, 'r') 
plt.xlabel('Time in seconds')
plt.ylabel('Bits per second')
plt.savefig('%s.jpg' %"rtt_based_rates")

plt.show()
