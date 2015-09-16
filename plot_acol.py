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
  if(xy[0]=="control"):
    qid = xy[8]
    if(qid not in x):
      x[qid] = []
      y[qid] = []

    x[qid].append(float(xy[6]))
    y[qid].append(xy[7]);

plt.figure(1)
plt.title("Waiting times of control packets")

for key in x:
  plt.plot(x[key], y[key], label=key) 
plt.legend(loc='upper right')
#plt.plot(xaxis, x, 'r') 
plt.xlabel('Time in seconds')
plt.ylabel('nanoseconds')
plt.savefig('%s.jpg' %"control_packets_waits")

plt.show()
