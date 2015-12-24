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

    if(flow_id not in dtimes):
      dtimes[flow_id] = []
      drates[flow_id] = []

    dtimes[flow_id].append(t1)
    drates[flow_id].append(rate)

  if(xy[0] == "QueueStats"):
    queue_id = xy[1]
    qtime = float(xy[2])
    qsize = float(xy[3])
    qfifosize = float(xy[4])
    qprice = float(xy[5])


    if(queue_id not in qtimes):
      qtimes[queue_id] = []
      qsizes[queue_id] = []
      qprices[queue_id] = []


    qtimes[queue_id].append(qtime)
    qsizes[queue_id].append(qsize)
    qprices[queue_id].append(qprice)


colors = ['r','b','g', 'm', 'c', 'y','k']

plt.figure(1);
plt.title("Sending rates")
i=0
for key in dtimes:
  plt.plot(dtimes[key], ewma(drates[key], 1.0), colors[i], label=str(key))
  i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Rate in Mbps')
plt.title('%s' %(pre) )
plt.legend(loc='upper right')
plt.savefig('%s.%s.png' %(pre,"rates"))

plt.draw()

plt.figure(2)
plt.title("QueuePrices")
i=0
for key in qprices:
#  if(key == "0_0_1" or key == "2_2_0" or key == "3_3_0" or key == "1_1_4" or key == "1_1_5"):
#  if(key == "7_7_0" or key == "0_0_1" or key == "1_1_3" or key == "1_1_2" or key == "2_2_10" or key=="3_3_4" or key=="4_4_12" or key=="6_6_0"):
      plt.plot(qtimes[key], qprices[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.xlabel('Time in seconds')
plt.ylabel('Queue Price')
plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_prices"))
plt.draw()


plt.show()

f.close()


