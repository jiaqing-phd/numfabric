import sys
import os
import matplotlib.pyplot as plt;

f = open(sys.argv[1]+".out")
pre = sys.argv[1];
dir = sys.argv[1]

if not os.path.exists(dir):
  os.makedirs(dir)

colors = ['r','b','g', 'm', 'c', 'y','k']
xy = []
x = {}
y = {}

qutiltimes = {}
qutils = {}
qminresidues = {}

qoccups0 = {}
qoccutimes0 = {}

qoccups1 = {}
qoccutimes1 = {}

wfqws = {}

rtime = []
trate = []

stimes = {}
srates = {}
dtimes = {}
drates = {}
crates = {}

qtimes = {}
qsizes = {}
fifoqsize = {}
qprices = {}

vtimes = {}
vtime_ts = {}

q0time_ts = {}
q1time_ts = {}
q0dlines = {}
q1dlines = {}

nprice_time = {}
nprices = {}
residues = {}

total_capacity=10000*2.0;

def ewma(values, g=1.0/8):
    ret = []
    prev = 0
    for v in values:
        prev = prev * (1.0 - g) + v * g
        ret.append(prev)
    return ret

num_flows=1

itimes = {}
iarrivals = {}
irtts = {}

for line in f:
  l1 = line.rstrip();
  xy = l1.split(' ');

  if(xy[0] == "QOCCU"):
    findi = xy[3]
    queue_id = xy[8]
    if(queue_id == "2_2_0"):
      if(findi not in qoccutimes0):
        qoccutimes0[findi] = []
      qoccutimes0[findi].append(float(xy[1]))
      if(findi not in qoccups0):
        qoccups0[findi] = []
      qoccups0[findi].append(int(xy[5]))

    if(queue_id == "1_1_3"):
      if(findi not in qoccutimes1):
        qoccutimes1[findi] = []
      qoccutimes1[findi].append(float(xy[1]))
      if(findi not in qoccups1):
        qoccups1[findi] = []
      qoccups1[findi].append(int(xy[5]))

  if(xy[0] == "processRate"):
    nid = int(xy[6])
    iarrival = float(xy[12])
    itime = float(xy[13])
    irtt = float(xy[17])
  
    if(nid not in itimes):
      itimes[nid] = []
      iarrivals[nid] = []
      irtts[nid] = []
    itimes[nid].append(itime)
    iarrivals[nid].append(iarrival) 
    irtts[nid].append(irtt)

  if(xy[0] == "NETW_PRICE"):
    nid = int(xy[6])
    if(nid == 0):
      continue
    t = float(xy[1])
    n_price = float(xy[10])
    resi = float(xy[12])
    wfqw = float(xy[14]) 
    if(nid not in nprice_time):
      nprice_time[nid] = []
      nprices[nid] = []
      residues[nid] = []
      wfqws[nid] = []
    nprice_time[nid].append(t)   
    nprices[nid].append(n_price)
    residues[nid].append(resi)
    wfqws[nid].append(wfqw)

  if(len(xy) > 1 and xy[0]=="num_flows"):
    num_flows=int(xy[1])
  if(xy[0] == "Rate"):
    sflow_id=int(xy[2])
    st1 = float(xy[3])
    srate=float(xy[4])

    if(sflow_id not in stimes):
      stimes[sflow_id] = []
      srates[sflow_id] = []

    stimes[sflow_id].append(st1)
    srates[sflow_id].append(srate)
  
  if(xy[0] == "DestRate"):
    flow_id=int(xy[2])
    t1 = float(xy[3])
    rate=float(xy[4])
    short_rate = float(xy[5])

    #if(flow_id not in dtimes and (flow_id == 1 or flow_id == 2 or flow_id ==3 or flow_id ==4)):
    if(flow_id not in dtimes):
      dtimes[flow_id] = []
      drates[flow_id] = []
      crates[flow_id] = []
    #if((flow_id == 1 or flow_id == 2 or flow_id == 3 or flow_id == 4)): 
    dtimes[flow_id].append(t1)
    drates[flow_id].append(rate)
    crates[flow_id].append(short_rate)
  if(len(xy)> 2 and xy[1] == "TotalRate"):
    rtime.append(float(xy[0]))
    trate.append(float(xy[2]))

  if(len(xy) > 2 and xy[1] == "Queue_id"):
    tn = xy[0]
    qid = xy[2]
    min_residue = float(xy[18])
    utilization = float(xy[8])
    
    if(qid not in qutiltimes):
      qutiltimes[qid] = []
      qminresidues[qid] = []
      qutils[qid] = []
  
    qutiltimes[qid].append(tn)
    qminresidues[qid].append(min_residue)
    qutils[qid].append(utilization)

  if(xy[0] == "QueueStats"):
    queue_id = xy[1]
    qtime = float(xy[2])
    qsize = float(xy[3])
    qfifosize = float(xy[4])
    qprice = float(xy[5])
    
    
    if(queue_id not in qtimes):
      qtimes[queue_id] = []
      qsizes[queue_id] = []
      fifoqsize[queue_id] = []
      qprices[queue_id] = []
      
    
    qtimes[queue_id].append(qtime)
    qsizes[queue_id].append(qsize)
    fifoqsize[queue_id].append(qfifosize)
    qprices[queue_id].append(qprice)

  if(xy[0]=="virtualtime"):
    vtime=float(xy[5])
    s=(xy[3])
    time_t=float(xy[4])

    if(s not in vtimes):
      vtimes[s] = []
      vtime_ts[s] = []
    (vtimes[s]).append(vtime)
    (vtime_ts[s]).append(time_t)

  

  if(xy[0] == "packetdeadline"):
    time_t = float(xy[1])
    dline = float(xy[2])
    nodeid=int(xy[6])
    fkey=xy[7]
    
    if(nodeid == 0):
      if(fkey not in q0dlines):
        q0dlines[fkey] = []
        q0time_ts[fkey] = []
      q0dlines[fkey].append(dline)
      q0time_ts[fkey].append(time_t)
        
    if(nodeid == 1):
      if(fkey not in q1dlines):
        q1dlines[fkey] = []
        q1time_ts[fkey] = []
      q1dlines[fkey].append(dline)
      q1time_ts[fkey].append(time_t)
       
   
##### PLOTS ##### 
plt.figure(1)
plt.title("QueueOccupancy")

i=0
for key in qsizes:
#    if(key == "0_0_1" or key == "2_2_0" or key == "3_3_0" or key == "1_1_4" or key == "1_1_5"):
      plt.plot(qtimes[key], ewma(qsizes[key], 1.0), colors[i], label=str(key)) 
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1") 
plt.xlabel('Time in seconds')
plt.ylabel('Queue occupancy in Bytes')
plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_occupancy"))
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

plt.figure(3)
plt.title("Larger EWMA constant")
i=0
for key in dtimes:
  plt.plot(dtimes[key], ewma(drates[key], 1.0), colors[i], label=str(key)) 
  i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Rates in Mbps')
plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"large_ewma"))

plt.draw()

plt.figure(4)
plt.title("Small EWMA constant")
i=0
for key in dtimes:
  plt.plot(dtimes[key], ewma(crates[key], 1.0), colors[i], label=str(key)) 
  i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Rates in Mbps')
plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"small_ewma"))

plt.draw()


#plt.figure(4)
#plt.title("switch0 deadlines")
#for f in q0dlines:
#  plt.plot(q0time_ts[f], q0dlines[f], label=str(f))
#plt.xlabel("Time in seconds")
#plt.ylabel("Time in NanoSeconds")
#plt.legend(loc='upper right')
#plt.savefig("%s/%s.png" %(pre,"q0_deadlines"))
#plt.draw()

#plt.figure(5)
#plt.title("QueueVirtualtimes")
#i=0
#for key in vtimes:
#    if(key != "1000"):
#      plt.plot(vtime_ts[key], vtimes[key], colors[i], label=str(key)) 
#      i = (i+1)%len(colors)
#plt.xlabel('Time in seconds')
#plt.ylabel('Queue Virtual time')
#plt.legend(loc='lower right')
#plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_virtualtime"))
#plt.draw()

plt.figure(6)
plt.title("FlowAggregatePrices")
i=0
for key in nprice_time:
      plt.plot(nprice_time[key], nprices[key], colors[i], label=str(key)) 
      i = (i+1)%len(colors)
plt.xlabel('Time in seconds')
plt.ylabel('Flow Aggregate Prices')
plt.legend(loc='lower right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"flow_prices"))
plt.draw()



#plt.figure(7)
#plt.title("FlowResidues")
#i=0
#for key in nprice_time:
#        plt.plot(nprice_time[key], residues[key], colors[i], label=str(key)) 
#        i = (i+1)%len(colors)
#plt.xlabel('Time in seconds')
#plt.ylabel('Residues')
#plt.ylim(-0.0004, 0.0004)
#plt.legend(loc='lower right')
#plt.savefig('%s/%s.%s.png' %(pre,pre,"residues"))
#plt.draw()

#plt.figure(8)
#plt.title("Queue Minimum residue")
#i=0
#for key in qutiltimes:
#  if(key== "0_0_1"):
#      plt.plot(qutiltimes[key], qminresidues[key], colors[i], label=str(key)) 
#      i = (i+1)%len(colors)
#plt.xlabel('Time in seconds')
#plt.ylabel('Minimum residue')
#plt.legend(loc='lower right', prop={'size':6})
#plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_minresidues"))
#plt.draw()

#plt.figure(9)
#plt.title("Queue Utilization")
#i=0
#for key in qutiltimes:
#  if(key== "0_0_1"):
#      plt.plot(qutiltimes[key], qutils[key], colors[i], label=str(key)) 
#      i = (i+1)%len(colors)
#plt.xlabel('Time in seconds')
#plt.ylabel('Utilization')
#plt.legend(loc='lower right', prop={'size':6})
#plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_utilization"))
#plt.draw()

plt.figure(9)
plt.title("RTTs")
i=0
for key in itimes:
  plt.plot(itimes[key], irtts[key], colors[i], label=str(key)) 
  i = (i+1)%len(colors)
plt.xlabel('Time in seconds')
plt.ylabel('RTT')
plt.legend(loc='lower right', prop={'size':8})
plt.savefig('%s/%s.%s.png' %(pre,pre,"rtt"))
plt.draw()

plt.figure(11)
plt.title("Inter-arrivals")
i=0
for key in itimes:
  plt.plot(itimes[key], iarrivals[key], colors[i], label=str(key)) 
  i = (i+1)%len(colors)
plt.xlabel('Time in seconds')
plt.ylabel('Inter-arrival times')
plt.legend(loc='lower right', prop={'size':6})
plt.savefig('%s/%s.%s.png' %(pre,pre,"inter_arrivals"))
plt.draw()

plt.figure(10)
plt.title("FlowWeights")
i=0
for key in nprice_time:
  plt.plot(nprice_time[key], wfqws[key], colors[i], label=str(key)) 
  i = (i+1)%len(colors)
plt.xlabel('Time in seconds')
plt.ylabel('Weights')
plt.legend(loc='lower right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"WFQ_Weights"))
plt.draw()
#cwndx = {}
#cwndy = {}
#for fid in range(1,17):
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
#      cwndy[fid].append(xy1[2])

#plt.figure(13)
#plt.title("Congestion Windows")
#
#i=0
#for key in cwndx:
#  print("plotting flow id %d"%key)
#  
#  plt.plot(cwndx[key], cwndy[key], colors[i], label=`key`)
#  i = (i+1)%len(colors)
#plt.xlabel('Time in seconds')
#plt.ylabel('Congestion windows')
#plt.legend(loc='lower right')
#plt.savefig('%s/%s.%s.png' %(pre,pre,"cwnd"))

#plt.draw()

i=0
plt.figure(5)
plt.title("Queue flow-level occupancy - Switch0")
for key in qoccups0:
  plt.plot(qoccutimes0[key], qoccups0[key], colors[i], label=str(key))
  i= (i+1)% len(colors)


plt.legend(loc='upper right')
plt.xlabel('Time in seconds')
plt.ylabel('Number of Packets')
plt.savefig('%s/%s.%s.jpg' %(pre,pre,"flow_queue_occupancy_switch0"))
plt.draw()

#i=0
#plt.figure(6)
#plt.title("Queue flow-level occupancy - Switch1")
#for key in qoccups1:
#  plt.plot(qoccutimes1[key], qoccups1[key], colors[i], label=str(key))
#  i= (i+1)% len(colors)


#plt.legend(loc='upper right')
#plt.xlabel('Time in seconds')
#plt.ylabel('Number of Packets')
#plt.savefig('%s/%s.%s.jpg' %(pre,pre,"flow_queue_occupancy_switch1"))
#plt.draw()
plt.show()
