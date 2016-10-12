#!/usr/bin/python

import matplotlib.pyplot as plt
import sys
import os
import pickle
import subprocess

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
sink_times = {}
sink_diff = {}
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

  if(len(xy) > 5):
    if(xy[3] == "DCTCP_DEBUG"):
      dctcp_nid = int(xy[10])
      dctcp_time = float(xy[0])
      dctcp_alpha = float(xy[7])
      if(dctcp_nid not in dctcp_times):
        dctcp_times[dctcp_nid] = []
        dctcp_alphas[dctcp_nid] = []
      (dctcp_times[dctcp_nid]).append(dctcp_time)
      (dctcp_alphas[dctcp_nid]).append(dctcp_alpha) 
#  if(xy[0]=="sinkdata"):
#      fid = int(xy[3])
#      if (fid not in sink_times):
#          sink_times[fid] = []
#          sink_diff[fid] = []
#      sink_times[fid].append(xy[1])
#      sink_diff[fid].append(float(xy[10]))

  if(xy[0] == "DestRate"):
    flow_id=int(xy[2])
    t1 = float(xy[3])
    rate=float(xy[4])
    ideal=float(xy[5])
    epoch = int(xy[7])
    #if( epoch > 1):
    #    break
    if(flow_id not in dtimes):
      dtimes[flow_id] = []
      drates[flow_id] = []

    #drates[flow_id].append(rate)
    #if(abs((rate-ideal)/ideal) > 0.1):
    #if(epoch == 25 and flow_id == 974):
    dtimes[flow_id].append(t1)
    drates[flow_id].append(rate)
    

colors = ['r','b','g', 'm', 'c', 'y','k']


plt.figure(1);
rates_str=pre+"_rates"
plt.title(rates_str)
i=0
for key in dtimes:
      plt.plot(dtimes[key], drates[key], colors[i], label=str(key))
      i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Rate in Mbps')
plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"rates"))

plt.draw()

plt.figure(2);
tstr = pre+"_datasentconverge"
plt.title(tstr)
i=0
for key in sink_times:
      plt.plot(sink_times[key], sink_diff[key], colors[i], label=str(key))
      i = (i+1)%len(colors)

plt.xlabel('Time in seconds')
plt.ylabel('Percentage diff to ideal')
plt.legend(loc='upper right')
plt.savefig('%s/%s.%s.png' %(pre,pre,"datasentdiff"))
plt.draw()


#cwndx = {}
#cwndy = {}
#ls_lines = subprocess.check_output(['ls']).splitlines()
#for cwnd_fname in ls_lines:
#  if(sys.argv[1] in cwnd_fname and "cwnd" in cwnd_fname):
#      cwnd1 = cwnd_fname
#      print("opening file %s" %cwnd1)
#      if(os.path.exists(cwnd1)):
#        fid_list = cwnd1.split(".")
#        fid=int(fid_list[len(fid_list)-1])
#        
#        f1 = open(cwnd1)
#        for line in f1:
#            L = line.rstrip();
#            xy1 = L.split('\t');
#            if(fid not in cwndx):
#                cwndx[fid] = []
#                cwndy[fid] = []
#            cwndx[fid].append(xy1[0])
#            cwndy[fid].append(xy1[1])
#plt.figure(8)
#plt.title("Congestion Windows")
#
#i=0
#for key in cwndx:
#      print("plotting flow id %s"%key)
#      plt.plot(cwndx[key], cwndy[key], colors[i], label=`key`)
#      i = (i+1)%len(colors)
#plt.xlabel('Time in seconds')
#plt.ylabel('Congestion windows')
#plt.legend(loc='lower right')
#plt.draw()
#plt.savefig('%s/%s.%s.png' %(pre,pre,"cwnd"))

plt.show()

f.close()


