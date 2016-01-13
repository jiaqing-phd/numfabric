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
qminresidues = {}

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

def SecondTerm(prices, utils, eta=10.0):
    ret = []
    prev = 0
    index=0
    for v in utils:
        curr = -prev * eta * max(0,v)
        prev= prices[index]
        index+=1
        ret.append(curr)
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

  if(xy[0] == "QUEUESTATS"):
    qtime = float(xy[1])
    queue_id = xy[2]
    qprice = float(xy[3])
    qutil = float(xy[4])
    qmin_residue = float(xy[5])
    qsize = int(xy[6])


    if(queue_id not in qtimes):
      qtimes[queue_id] = []
      qprices[queue_id] = []
      qutils[queue_id] = []
      qminresidues[queue_id] = []
      qsizes[queue_id] = []


    qtimes[queue_id].append(qtime)
    qprices[queue_id].append(qprice)
    qutils[queue_id].append(qutil)
    qminresidues[queue_id].append(qmin_residue)
    qsizes[queue_id].append(qsize)


colors = ['r','b','g', 'm', 'c', 'y','k','#fedcba','#abcdef','#ababab','#badaff','#deadbe','#bedead','#afafaf','#8eba42','#e5e5e5','#6d904f']


#dinesh
eta=10.0
second_term = {}
for key in qprices:
    second_term[key]=SecondTerm(qprices[key],qutils[key],eta)


j=1;
for key in qtimes: 
  i=0
  plt.figure(j)
  plt.plot(qtimes[key], ewma(qprices[key], 1.0), color=colors[i], label=str(" actual price "))
  i = (i+1)%len(colors)
  new_vec=[0.0];
  new_vec.extend(ewma(qprices[key],1.0))
  del new_vec[-1] 
  plt.plot(qtimes[key], new_vec, color=colors[i], label=str(" prev price "))
  i = (i+1)%len(colors)
  plt.plot(qtimes[key], ewma(second_term[key], 1.0), color=colors[i], label=str(" util contribution" ))
  i = (i+1)%len(colors)
  plt.plot(qtimes[key], ewma(qminresidues[key], 1.0), color=colors[i], label=str(" min residue contri  "))
  i = (i+1)%len(colors)
  j+=1
  plt.xlabel('Time in seconds')
  plt.ylabel('Rate in Mbps')
  plt.title('%s_%s' %(pre, key) )
  plt.legend(loc='best')
  plt.savefig('%s/%s.%s_%d.png' %(pre,pre,"rates",j))
  plt.close()
i=0
for key in qtimes: 
  plt.figure(j)
  plt.plot(qtimes[key], qsizes[key], color=colors[i], label=str(" queue size "))
  j+=1
  plt.xlabel('Time in seconds')
  plt.ylabel('Queue size in bytes')
  plt.title('%s_%s' %(pre, key) )
  plt.legend(loc='best')
  plt.savefig('%s/%s.%s_%d.png' %(pre,pre,"qsizes",j))
  plt.close()

# end DINESH
i=0
j=1;
for key in dtimes:
  plt.plot(dtimes[key], ewma(drates[key], 1.0), color=colors[i], label=str(key))
  i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Rate in Mbps')
plt.title('%s_rates' %(pre) )
#plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"rates"))
plt.draw()

hosts = list(xrange(144))
leafs = list(xrange(144,153))

plt.figure(2)
plt.title('%s_edgelink_prices' %(pre) )
i=0
for key in qprices:
    parts = key.split('_');
    print(parts)
    if((int(parts[1]) in hosts) or (int(parts[2]) in hosts)):
      print("edge key %s" %key)
      plt.plot(qtimes[key], qprices[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.xlabel('Time in seconds')
plt.ylabel('Queue Price Edge')
#plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_prices_edge"))
plt.draw()

plt.figure(5)
plt.title('%s_leaflink_prices' %(pre) )
i=0
for key in qprices:
    parts = key.split('_');
    if(not((int(parts[1]) in hosts) or (int(parts[2]) in hosts))):
      print("leaf key %s" %key)
      plt.plot(qtimes[key], qprices[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.xlabel('Time in seconds')
plt.ylabel('Queue Price')
#plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_prices"))
plt.draw()

plt.figure(3)
plt.title('%s_leaflink_minresidues' %(pre) )
i=0
for key in qprices:
    parts = key.split('_');
    if(not((int(parts[1]) in hosts) or (int(parts[2]) in hosts))):
      plt.plot(qtimes[key], qminresidues[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.xlabel('Time in seconds')
plt.ylabel('Queue MinResidues')
#plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_minresidues_leaf"))
plt.draw()

plt.figure(7)
plt.title('%s_edgelink_minresidues' %(pre) )
i=0
for key in qprices:
    parts = key.split('_');
    if((int(parts[1]) in hosts) or (int(parts[2]) in hosts)):
      plt.plot(qtimes[key], qminresidues[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.xlabel('Time in seconds')
plt.ylabel('Queue MinResidues')
#plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_residues_edge"))
plt.draw()

plt.figure(4)
plt.title('%s_edgelink_utils' %(pre) )
i=0
for key in qprices:
    parts = key.split("_")
    if((int(parts[1]) in hosts) or (int(parts[2]) in hosts)):
      plt.plot(qtimes[key], qutils[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.ylim(0,1.2)
plt.xlabel('Time in seconds')
plt.ylabel('Queue Utils')
#plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_utils_edge"))
plt.draw()

plt.figure(6)
plt.title('%s_leaflink_utils' %(pre) )
i=0
for key in qprices:
    parts = key.split("_")
    if((int(parts[1]) in hosts) or (int(parts[2]) in hosts)):
      plt.plot(qtimes[key], qutils[key], colors[i], label=str(key))
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1")
plt.ylim(0,1.2)
plt.xlabel('Time in seconds')
plt.ylabel('Queue Rates')
#plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_utils_leaf"))
plt.draw()

#plt.show()

f.close()


