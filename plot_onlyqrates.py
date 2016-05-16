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
  if(len(xy) > 1 and xy[0]=="num_flows"):
    num_flows=int(xy[1])
  
  if(xy[0] == "Queue" and xy[1] == "size"):
    nid = int(xy[5])
    if(nid not in x):
        x[nid] = []
        y[nid] = []
    (x[nid]).append(float(xy[2]));
    (y[nid]).append(float(xy[3]));
  if(xy[0] == "RatePrio"):
    flow_id=int(xy[2])
    t1 = float(xy[3])
    rate=float(xy[4])
    prio= float(xy[5])

    if(flow_id not in dtimes):
      dtimes[flow_id] = []
      drates[flow_id] = []
      dprios[flow_id] = []

    dtimes[flow_id].append(t1)
    drates[flow_id].append(rate)
    dprios[flow_id].append(prio)
  if(len(xy)> 2 and xy[1] == "TotalRate"):
    rtime.append(float(xy[0]))
    trate.append(float(xy[2])/total_capacity)


  if(xy[0] == "QueueStats"):
    queue_id = xy[1]
    qtime = float(xy[2])
    qsize = float(xy[3])
    qprice = float(xy[4])
    
    if(queue_id not in qtimes):
      qtimes[queue_id] = []
      qsizes[queue_id] = []
      qprices[queue_id] = []
    
    qtimes[queue_id].append(qtime)
    qsizes[queue_id].append(qsize)
    qprices[queue_id].append(qprice)

  if(xy[0] == "QueueStats1"):
    queue_id = int(xy[1])
    qtime = float(xy[2])
    qfid = int(xy[3])
    qfdeadline = float(xy[4])
  
    if(queue_id == 0):
      if(qfid not in q0times):
        q0times[qfid] = []
        q0deadlines[qfid] = []
      q0times[qfid].append(qtime)
      q0deadlines[qfid].append(qfdeadline)
    
    
#plt.figure(1)
#plt.title("QueueOccupancy")

colors = ['r','b','g', 'm', 'c', 'y','k']

plt.figure(1)
plt.title("Sending rates")
i=0
for key in dtimes:
  plt.plot(dtimes[key], ewma(drates[key], 1.0), colors[i], label=str(key)) 
  i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Fraction of total capacity')
plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.jpg' %(pre,pre,"load"))

plt.draw()


plt.figure(2)
plt.title("Queue Size")
for f in qtimes:
  plt.plot(qtimes[f], qsizes[f], label=str(f))
plt.xlabel("Time in seconds")
plt.ylabel("Bytes")
plt.legend(loc='upper right')
plt.savefig("%s/%s.png" %(pre,"queue_sizes"))
plt.draw()

plt.show()

f.close()

#cwndx = {}
#cwndy = {}
#for fid in range(0,num_flows):
#  cwnd1 = sys.argv[1]+".cwnd."+`fid`
#  print("opening file %s" %cwnd1)
#  if(os.path.exists(cwnd1)):
#    f1 = open(cwnd1)
#    for line in f1:
#      L = line.rstrip();
#      xy1 = L.split('\t');
#      if(fid not in cwndx):
#        cwndx[fid] = []
#        cwndy[fid] = []
#      cwndx[fid].append(xy1[0])
#      cwndy[fid].append(xy1[1])

#plt.figure(8)
#plt.title("Congestion Windows")

#i=0
#for key in cwndx:
#  print("plotting flow id %d"%key)
#  plt.plot(cwndx[key], cwndy[key], colors[i], label=`key`)
#  i = (i+1)%len(colors)
#plt.xlabel('Time in seconds')
#plt.ylabel('Congestion windows')
#plt.legend(loc='lower right')
#plt.savefig('%s/%s.%s.jpg' %(pre,pre,"cwnd"))

#plt.draw()

