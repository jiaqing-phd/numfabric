import sys
import os
import matplotlib.pyplot as plt;

colors = ['r','b','g', 'm', 'c', 'y','k']
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

stimes = {}
srates = {}
dtimes = {}
drates = {}
crates = {}


start_times = {}
finish_times = {}
stime_xaxis = {}
finish_xaxis = {}

total_capacity=10000*2.0;

qtimes = {}
qsizes = {}
qprices = {}
fifoqsize = {}

qwaittimes = {}
qfwaits  ={}


def ewma(values, g=1.0/8):
    ret = []
    prev = 0
    for v in values:
        prev = prev * (1.0 - g) + v * g
        ret.append(prev)
    return ret

num_flows=1

for line in f:
  l1 = line.rstrip();
  xy = l1.split(' ');
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
    #csfq_rate = float(xy[5])

    #if(flow_id not in dtimes and (flow_id == 1 or flow_id == 2 or flow_id ==3 or flow_id ==4)):
    if(flow_id not in dtimes):
      dtimes[flow_id] = []
      drates[flow_id] = []
      #crates[flow_id] = []
    #if((flow_id == 1 or flow_id == 2 or flow_id == 3 or flow_id == 4)): 
    dtimes[flow_id].append(t1)
    drates[flow_id].append(rate)
    #crates[flow_id].append(csfq_rate)
  if(len(xy)> 2 and xy[1] == "TotalRate"):
    rtime.append(float(xy[0]))
    trate.append(float(xy[2]))

  if(xy[0] == "start_time"):
    fid = int(xy[2])
    stime = float(xy[3])
    curtime = float(xy[4])
    if(fid not in start_times):
      start_times[fid] = []
      stime_xaxis[fid] = []
    start_times[fid].append(stime)
    stime_xaxis[fid].append(curtime)
 
  if(xy[0] == "finish_time"):
    fid = int(xy[2])
    stime = float(xy[3])
    curtime = float(xy[4])
    if(fid not in finish_times):
      finish_times[fid] = []
      finish_xaxis[fid] = []
    finish_times[fid].append(stime)
    finish_xaxis[fid].append(curtime)
 
  # example FIFO_QueueStats 
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

  if(len(xy) > 5):
    if(xy[3] == "DCTCP_DEBUG"):
      dctcp_nid = xy[10]
      dctcp_time = float(xy[0])
      dctcp_alpha = float(xy[7])
      if(dctcp_nid not in dctcp_times):
        dctcp_times[dctcp_nid] = []
        dctcp_alphas[dctcp_nid] = []
      (dctcp_times[dctcp_nid]).append(dctcp_time)
      (dctcp_alphas[dctcp_nid]).append(dctcp_alpha) 

  vtimes = {}
  vtime_ts = {}

  if(xy[0]=="virtualtime"):
    vtime=float(xy[5])
    s=int(xy[3])
    time_t=float(xy[4])

    if(s not in vtimes):
      vtimes[s] = []
      vtime_ts[s] = []
    (vtimes[s]).append(vtime)
    (vtime_ts[s]).append(time_t)

  
  q0time_ts = {}
  q1time_ts = []
  q0dlines = {}
  q1dlines = {}

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
      q1time_ts.append(time_t)
      if(fkey not in q1dlines):
        q1dlines[fkey] = []
      q1dlines[fkey].append(dline)
       
    

#  if(xy[0] == "QueueStats1"):
#    queue_id = xy[1]
#    qtime = float(xy[2])
#    qfid = int(xy[3])
#    qfdeadline = float(xy[4])
#    qvirtual_time = float(xy[5])
#    qnid = int(xy[5])

#    if(queue_id == "0_0_1"):
#      if(qfid not in q0times):
#        q0times[qfid] = []
#        q0deadlines[qfid] = []
#      q0times[qfid].append(qtime)
#      q0deadlines[qfid].append(qfdeadline)
#      qvirtual_times.append(qvirtual_time)
#      qvirtualtimes_times.append(qtime)
  
  if(xy[0] == "QWAIT"):
    qtime = float(xy[2])
    qfid = xy[3]
    qfwait = float(xy[5])
    queue_id = xy[1]

    if(queue_id == "0_0_1"):
      if(qfid not in qwaittimes):
        qwaittimes[qfid] = []
        qfwaits[qfid] = []
      qwaittimes[qfid].append(qtime)
      qfwaits[qfid].append(qfwait)
    
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
plt.legend(loc='lower right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_occupancy"))
plt.draw()

plt.figure(15)
plt.title("QueuePrices")
i=0
for key in qprices:
#    if(key == "0_0_1" or key == "2_2_0" or key == "3_3_0" or key == "1_1_4" or key == "1_1_5"):
      plt.plot(qtimes[key], qprices[key], colors[i], label=str(key)) 
      i = (i+1)%len(colors)
#plt.plot(qx1, qy1, 'k', label="switch1") 
plt.xlabel('Time in seconds')
plt.ylabel('Queue Price')
plt.legend(loc='lower right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_prices"))
plt.draw()

#plt.figure(3)
#plt.title("Sending rates at sender")
#i=0
#for key in stimes:
#  plt.plot(stimes[key], ewma(srates[key], 1.0), colors[i]) 
#  i = (i+1)%len(colors)

#plt.xlabel('Time in seconds')
#plt.ylabel('Rates in Mbps')
#plt.legend(loc='upper right')
#plt.savefig('%s/%s.%s.png' %(pre,pre,"rates"))

#plt.draw()

plt.figure(6)
plt.title("Sending rates at destination")
i=0
for key in dtimes:
  plt.plot(dtimes[key], ewma(drates[key], 1.0), colors[i], label=str(key)) 
  i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Rates in Mbps')
plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"destination_rates"))

plt.draw()


#plt.figure(7)
#plt.title("Sending rates at destination (per-packet exponential averaged)")
#i=0
#for key in dtimes:
#  plt.plot(dtimes[key], ewma(crates[key], 1.0), colors[i]) 
#  i = (i+1)%len(colors)

#plt.xlabel('Time in seconds')
#plt.ylabel('Rates in Mbps')
#plt.legend(loc='upper right')
#plt.savefig('%s/%s.%s.png' %(pre,pre,"destination_rates_perpacket"))

#plt.draw()

plt.figure(1)
plt.title("switch0 deadlines")
for f in q0dlines:
  plt.plot(q0time_ts[f], q0dlines[f], label=str(f))
plt.xlabel("Time in seconds")
plt.ylabel("Time in NanoSeconds")
plt.legend(loc='upper right')
plt.savefig("%s/%s.png" %(pre,"q0_deadlines"))
plt.draw()

#plt.figure(5)
#plt.title("flow wait times at bottleneck link")
#for f in qwaittimes:
#  if(f == 0):
#    continue
#  plt.plot(qwaittimes[f], qfwaits[f], label=str(f))
#plt.xlabel("Time in seconds")
#plt.ylabel("Time in Nanoseconds")
#plt.legend(loc='upper right')
#plt.savefig("%s/%s.png" %(pre,"q0_waittimes"))
#plt.draw()


#plt.figure(11)
#plt.title("starttime and finish times at switch")
#for f in start_times:
#  plt.plot(stime_xaxis[f], start_times[f], label="start"+str(f))
#  plt.plot(stime_xaxis[f], finish_times[f], label="finish"+str(f), marker="x", markevery=1000)
#plt.xlabel("Time in seconds")
#plt.ylabel("Time in Nanoseconds")
#plt.legend(loc='upper right')
#plt.savefig("%s/%s.%s.png" %(pre,pre,"q0_starttimes"))
#plt.draw()

#plt.figure(5)
#plt.title("DCTCP alpha")
#plt.xlabel('Time in seconds')
#plt.ylabel("alpha")
#for key in dctcp_times:
#  print key
#  plt.plot(dctcp_times[key], dctcp_alphas[key], label=str(key))
#plt.legend(loc='upper right')
#plt.savefig("%s/%s.%s.png" %(pre,pre,"dctcp_alphas"))
#plt.draw()

plt.figure(5)
plt.title("QueueVirtualtimes")
i=0
for key in vtimes:
    print vtimes
    plt.plot(vtime_ts[key], vtimes[key], colors[i], label=str(key)) 
    i = (i+1)%len(colors)
plt.xlabel('Time in seconds')
plt.ylabel('Queue Virtual time')
plt.legend(loc='lower right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_virtualtime"))
plt.draw()

cwndx = {}
cwndy = {}
for fid in range(1,num_flows+1):
  cwnd1 = sys.argv[1]+".cwnd."+`fid`
  print("opening file %s" %cwnd1)
  if(os.path.exists(cwnd1)):
    f1 = open(cwnd1)
    for line in f1:
      L = line.rstrip();
      xy1 = L.split('\t');
      if(fid not in cwndx):
        cwndx[fid] = []
        cwndy[fid] = []
      cwndx[fid].append(xy1[0])
      cwndy[fid].append(xy1[2])

plt.figure(9)
plt.title("Congestion Windows")

i=0
for key in cwndx:
  print("plotting flow id %d"%key)
  
  plt.plot(cwndx[key], cwndy[key], colors[i], label=`key`)
  i = (i+1)%len(colors)
plt.xlabel('Time in seconds')
plt.ylabel('Congestion windows')
plt.legend(loc='lower right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"cwnd"))

plt.draw()

plt.show()
