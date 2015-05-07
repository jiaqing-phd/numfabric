import sys
import os

import matplotlib.pyplot as plt

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
total_capacity=10000

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
#  if(len(xy) > 1 and xy[0]=="num_flows"):
#    num_flows=int(xy[1])
  
  if(xy[0] == "Rate"):
    flow_id=int(xy[2])
    t1 = float(xy[3])
    rate=float(xy[4])

    if(flow_id not in dtimes):
      dtimes[flow_id] = []
      drates[flow_id] = []

    dtimes[flow_id].append(t1)
    drates[flow_id].append(rate)
  if(len(xy)> 2 and xy[1] == "TotalRate"):
    rtime.append(float(xy[0]))
    trate.append(float(xy[2])/total_capacity)


  if(xy[0] == "QueueStats"):
    queue_id = xy[1]
    qtime = float(xy[2])
    qsize = float(xy[3])
    
    if(queue_id not in qtimes):
      qtimes[queue_id] = []
      qsizes[queue_id] = []
    
    qtimes[queue_id].append(qtime)
    qsizes[queue_id].append(qsize)

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

  if(xy[0] == "QueueStats1"):
    queue_id = xy[1]
    qtime = float(xy[2])
    qfid = int(xy[3])
    qfdeadline = float(xy[4])
    qnid = int(xy[5])

    if(qnid == 0):
      if(qfid not in q0times):
        q0times[qfid] = []
        q0deadlines[qfid] = []
      q0times[qfid].append(qtime)
      q0deadlines[qfid].append(qfdeadline)
  
    
plt.figure(1)
plt.title("QueueOccupancy")

colors = ['r','b','g', 'm', 'c', 'y','k']
i=0
for key in qsizes:
    plt.plot(qtimes[key], ewma(qsizes[key], 1.0), colors[i]) 
    i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1") 
plt.xlabel('Time in seconds')
plt.ylabel('Queue occupancy in Bytes')
plt.legend(loc='lower right')
plt.savefig('%s/%s.%s.jpg' %(pre,pre,"queue_occupancy"))
plt.draw()

plt.figure(2)
plt.title("Sum of all sending rates / total sending capacity")

plt.plot(rtime, trate, colors[i]) 
plt.xlabel('Time in seconds')
plt.ylabel('Fraction of total capacity')
plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.jpg' %(pre,pre,"load"))

plt.draw()


plt.figure(3)
plt.title("Sending rates")
i=0
for key in dtimes:
  plt.plot(dtimes[key], ewma(drates[key], 1.0), colors[i]) 
  i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Fraction of total capacity')
plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.jpg' %(pre,pre,"load"))

plt.draw()

plt.figure(4)
plt.title("flow deadlines at switch0")
for f in q0times:
  if(f == 0):
    continue
  plt.plot(q0times[f], q0deadlines[f], label=str(f))
plt.xlabel("Time in seconds")
plt.ylabel("Deadlines")
plt.legend(loc='upper right')
plt.savefig("%s/%s.png" %(pre,"q0_deadlines"))
plt.draw()

plt.figure(5)
plt.title("DCTCP alpha")
plt.xlabel('Time in seconds')
plt.ylabel("alpha")
if key in dctcp_times:
  plt.plot(dctcp_times[key], dctcp_alphas[key], label=str(key))
plt.legend(loc='upper right')
plt.savefig("%s/%s.%s.png" %(pre,pre,"dctcp_alphas"))
plt.draw()

plt.show()
