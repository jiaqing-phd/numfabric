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

    if(flow_id not in dtimes):
      dtimes[flow_id] = []
      drates[flow_id] = []

    dtimes[flow_id].append(t1)
    drates[flow_id].append(rate)

  if(xy[0] == "QueueStats"):
    queue_id = xy[1]
    qtime = float(xy[2])
    qsize = float(xy[3])
    qnodeid = float(xy[4])
    qprice = float(xy[5])
    qrate = float(xy[6])
    qutil = float(xy[7])


    if(queue_id not in qtimes):
      qtimes[queue_id] = []
      qsizes[queue_id] = []
      qprices[queue_id] = []
      qrates[queue_id] = []
      qutils[queue_id] = []


    qtimes[queue_id].append(qtime)
    qsizes[queue_id].append(qsize)
    qprices[queue_id].append(qprice)
    qrates[queue_id].append(qrate)
    qutils[queue_id].append(qutil)


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
plt.savefig('%s/%s.%s.png' %(pre,pre,"rates"))

plt.draw()

hosts = ["0","1","2","3","4","5","6","7"]
leafs = ["8","9","10","11"]

plt.figure(2)
plt.title("QueuePrices Edge")
i=0
for key in qprices:
    parts = key.split('_');
    print(parts)
    if((parts[1] in hosts) or (parts[2] in hosts)):
      print("edge key %s" %key)
      plt.plot(qtimes[key], qprices[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.xlabel('Time in seconds')
plt.ylabel('Queue Price Edge')
plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_prices_edge"))
plt.draw()

plt.figure(5)
plt.title("QueuePrices Leaf")
i=0
for key in qprices:
    parts = key.split('_');
    if(not((parts[1] in hosts) or (parts[2] in hosts))):
      print("leaf key %s" %key)
      plt.plot(qtimes[key], qprices[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.xlabel('Time in seconds')
plt.ylabel('Queue Price')
plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_prices"))
plt.draw()

plt.figure(3)
plt.title("QueueRates Leaf")
i=0
for key in qprices:
    parts = key.split('_');
    if(not((parts[1] in hosts) or (parts[2] in hosts))):
      plt.plot(qtimes[key], qrates[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.xlabel('Time in seconds')
plt.ylabel('Queue Rates')
plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_rates_leaf"))
plt.draw()

plt.figure(7)
plt.title("QueueRates Edge")
i=0
for key in qprices:
    parts = key.split('_');
    if((parts[1] in hosts) or (parts[2] in hosts)):
      plt.plot(qtimes[key], qrates[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.xlabel('Time in seconds')
plt.ylabel('Queue Rates')
plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_rates_edge"))
plt.draw()

plt.figure(4)
plt.title("QueueUtils Edge")
i=0
for key in qprices:
    parts = key.split("_")
    if((parts[1] in hosts) or (parts[2] in hosts)):
      plt.plot(qtimes[key], qutils[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.ylim(0,1.2)
plt.xlabel('Time in seconds')
plt.ylabel('Queue Utils')
plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_utils_edge"))
plt.draw()

plt.figure(6)
plt.title("QueueUtils Leaf")
i=0
for key in qprices:
    parts = key.split("_")
    if(not((parts[1] in hosts) or (parts[2] in hosts))):
      plt.plot(qtimes[key], qutils[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.ylim(0,1.2)
plt.xlabel('Time in seconds')
plt.ylabel('Queue Rates')
plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_utils_leaf"))
plt.draw()

plt.show()

f.close()


