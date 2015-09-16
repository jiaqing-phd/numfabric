#!/usr/bin/python

import matplotlib.pyplot as plt
import matplotlib as mp
import sys
import os

mp.rcParams.update({"font.size":22})
f = open(sys.argv[1]+".out")
pre = sys.argv[1];
dir = sys.argv[1]

if not os.path.exists(dir):
  os.makedirs(dir)

xy = []

def ewma(values, g=1.0/8):
    ret = []
    prev = 0
    for v in values:
        prev = prev * (1.0 - g) + v * g
        ret.append(prev)
    return ret

rtimes = {}
rrates = {}
rprio = {}
rprices = {}

dtimes = {}
drates = {}

qtimes = {}
qsizes = {}
qprices = {}

q0times = {}
q0deadlines = {}
q1times = {}
q1deadlines = {}
rincrements = {}


dctcp_alphas = {}
dctcp_times = {}


for line in f:
  l1 = line.rstrip();
  xy = l1.split(' ');

  if(xy[0] == "RatePrio"):
    flow_id=int(xy[2])
    t1 = float(xy[3])
    rate=float(xy[4])
    prio= float(xy[5])
    flow_price=float(xy[6])
    #incr = float(xy[7])

    if(flow_id not in rtimes):
      rtimes[flow_id] = []
      rrates[flow_id] = []
      rprio[flow_id] = []
      rprices[flow_id] = []
      #rincrements[flow_id] = []

    rtimes[flow_id].append(t1)
    rrates[flow_id].append(rate/1000.0)
    rprio[flow_id].append(prio)
    rprices[flow_id].append(flow_price)
    #rincrements[flow_id].append(incr)
    
  if(xy[0] == "DestRatePrio"):
    flow_id=int(xy[2])
    t1 = float(xy[3])
    rate=float(xy[4])

    if(flow_id not in dtimes):
      dtimes[flow_id] = []
      drates[flow_id] = []

    dtimes[flow_id].append(t1)
    drates[flow_id].append(rate/1000.0)

  if(xy[0] == "QueueStats"):
    queue_id = int(xy[1])
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
    
    if(queue_id == 1):
      if(qfid not in q1times):
        q1times[qfid] = []
        q1deadlines[qfid] = []
      q1times[qfid].append(qtime)
      q1deadlines[qfid].append(qfdeadline)

  if(len(xy) > 5):
    if(xy[3] == "DCTCP_DEBUG"):
      dctcp_nid = int(xy[2])
      dctcp_time = xy[0]
      dctcp_alpha = xy[7]
      if(dctcp_nid not in dctcp_times):
        dctcp_times[dctcp_nid] = []
      (dctcp_times[dctcp_nid]).append(dctcp_time)
      if(dctcp_nid not in dctcp_alphas):
        dctcp_alphas[dctcp_nid] = []
      (dctcp_alphas[dctcp_nid]).append(dctcp_alpha) 



#### Now, plotting
#plt.figure(11)
#plt.title("Dest Flow Rates")
#for f in dtimes:
#  plt.plot(dtimes[f], drates[f], label=str(f))
#plt.xlabel("Time in seconds")
#plt.ylabel("Rate in Mbps")
#plt.legend(loc='lower right')
#plt.savefig("%s/%s.png" %(pre,"dest_flow_rates"))
#plt.draw()

# optimals
markers=["s","d","p","v","+","*"]
optimal_value = {}
optimal_value[1]=[3720,3809,5000,5272,3109,4180]
optimal_value[2]=[3488,5238,1666,4393,5182,3488]
optimal_value[3]=[3488,5238,4285,6444,5528,3488]
optimal_value[4]=[2790,952,3333,333,1707,2325]
optimal_value[5]=[3720,3809,2380,3222,2764,4186]

xseries = {}
for i in range(1,6):
  cur_series = optimal_value[i]
  xseries[i] = []
  for xval in cur_series:
    xvalseries = [xval/1000.0]*1000
    xseries[i] = xseries[i] + xvalseries


colors = ['b', 'g', 'y', 'r', 'm', 'c']
opt_colors = ['b--', 'g--', 'y--', 'r--', 'm--', 'c--']
plt.figure(1)
i=0
for f in rtimes:
  if(f == 0):
    continue
  plt.plot(rtimes[f], xseries[f], opt_colors[i],linewidth=2)
  plt.plot(rtimes[f], rrates[f], colors[i], linewidth=2)
  i+=1
plt.ylim(0, 10)
plt.xlabel("Time in seconds")
plt.ylabel("Rate in Gbps")
plt.savefig("%s/%s.png" %(pre,"flow_rates"))
plt.draw()

plt.figure(2)
plt.title("Flow Prices reported at source")
for f in rtimes:
  if(f == 0):
    continue
  plt.plot(rtimes[f], rprices[f], label=str(f))
plt.xlabel("Time in seconds")
plt.ylabel("Network Price")
plt.legend(loc='lower right')
plt.savefig("%s/%s.png" %(pre,"flow_prices"))
plt.draw()
    
plt.figure(3)
plt.title("Queue Size")
for f in qtimes:
  if(f==3005):
    label_me="4"
  elif(f==1 or f==0 or f==2 or f==3 or f==4):
    label_me=str(f)
  
  if(f==1 or f==0 or f==2 or f==3 or f==4):
    plt.plot(qtimes[f], qsizes[f], label=label_me)
plt.xlabel("Time in seconds")
plt.ylabel("Bytes")
plt.legend(loc='lower right')
plt.savefig("%s/%s.png" %(pre,"queue_sizes"))
plt.draw()
     

plt.figure(5)
plt.title("link Prices reported at switch")
for f in qtimes:
  if(f==3005):
    label_me="4"
  elif(f==1 or f==0 or f==2 or f==3 or f==4):
    label_me=str(f)
  
  if(f==1 or f==0 or f==2 or f==3 or f==4):
    plt.plot(qtimes[f], qprices[f], label=label_me)
plt.xlabel("Time in seconds")
plt.ylabel("Link Price")
plt.legend(loc='lower right')
plt.savefig("%s/%s.png" %(pre,"link_prices"))
plt.draw()


plt.show()
