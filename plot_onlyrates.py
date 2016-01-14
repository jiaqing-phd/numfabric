#!/usr/bin/python

import matplotlib.pyplot as plt
import sys
import os

f = open(sys.argv[1]+".out")
pre = sys.argv[1];
dir = sys.argv[1]

if not os.path.exists(dir):
  os.makedirs(dir)

xy = []
x = {}
y = {}

rtime = []
trate = []

dtimes = {}
drates = {}
dprios = {}
total_capacity=1000000

qtimes = {}
qsizes = {}
qprices = {}
qrates = {}
qutils = {}

q0times = {}
q0deadlines = {}
q1times = {}
q1deadlines = {}
rincrements = {}
def ewma(values, g=1.0/8):
    ret = []
    prev = 0
    for v in values:
        prev = prev * (1.0 - g) + v * g
        ret.append(prev)
    return ret


for line in f:
  l1 = line.rstrip();
  xy = l1.split(' ');

  if(xy[0] == "DestRate"):
    flow_id=int(xy[2])
    t1 = float(xy[3])
    rate=float(xy[4])
    if( t1 > 2.2): 
        break
    if(flow_id not in dtimes):
      dtimes[flow_id] = []
      drates[flow_id] = []

    dtimes[flow_id].append(t1)
    drates[flow_id].append(rate)

colors = ['r','b','g', 'm', 'c', 'y','k']

plt.figure(1);
plt.title(pre)
i=0
for key in dtimes:
  plt.plot(dtimes[key], ewma(drates[key], 1.0), colors[i], label=str(key))
  i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Rate in Mbps')
plt.title('%s' %(pre) )
#plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"rates"))

plt.draw()


plt.show()

f.close()


