#!/usr/bin/python
import matplotlib.pyplot as plt
import sys
import os
import pickle
import subprocess
import numpy as np

pre = sys.argv[1];
dir = sys.argv[1]

if not os.path.exists(dir):
  os.makedirs(dir)

#sinkdata 1.03755 flowid 131 totalRx 117654864 epoch 1 ideal_rate 3127.76

epoch_time = 0.1 #50 ms
sample_time = 0.00005
window = 10

xy = []

def same_ballpark(cur_value, mean):
    if(abs(cur_value-mean) < math.ceil(0.1*mean)):
        #print("value %f mean %f value-mean %f ceil %f: MATCH" %(cur_value, mean, abs(cur_value-mean), math.ceil(0.1*mean)))
        return True
    else:
        #print("value %f mean %f value-mean %f ceil %f: NOMATCH" %(cur_value, mean, abs(cur_value-mean), math.ceil(0.1*mean)))
        return False

# window of few time-slots
def below_10(sts):
    start = 0
    end = window
    converged_time = len(sts)
    while(end < len(sts)):
        #print("CALCULATING MEAN FOR start %d end %d "%(start, end))
        index = 0
        while(abs(sts[start+index])<0.1 and index<window):
            #print("value %f index %d" %(sts[index], index))
            index +=1
        # Broke out of while, did we match 10?
        if(index >= window):
            converged_time = start
            return converged_time
        else:
            start = start+index+1
            end = start+window

    return (converged_time)



def get_flow_start_times(epoch_num):
    flow_start = {}

    time_now = (epoch_num-1) * epoch_time + 1.0
    #print("get_flow_start_times: checking till epoch %d till time %f" %(epoch_num, time_now))
    f = open(sys.argv[1]+".out")
    for line in f:
     l1 = line.rstrip();
     xy = l1.split(' ');


     if(xy[0] == "flow_start"):
         fid = int(xy[1])
         flow_start_time = float(xy[3])/1000000000.0
         if(flow_start_time > time_now):
             #print("flow_start %f higher than  time_now %f epoch %d" %(flow_start_time, time_now, epoch_num))
             break
         flow_start[fid] = flow_start_time
         #print("adding flow %d start_time %f" %(fid, flow_start[fid]))
     if(xy[0] == "flow_stop" and xy[9]=="weight"):
         fid = int(xy[1])
         #print xy
         flow_stop_time = float(xy[5])/1000000000.0
         if(flow_stop_time > time_now):
             #print("flow_stop %f higher than  time_now %f epoch %d" %(flow_stop_time, time_now, epoch_num))
             break

         #print("removing flow %d start_time %f" %(fid, float(xy[5])/1000000000.0))
         flow_start[fid] = 0.0

    return flow_start

def dead_flow(totalrx, fid, deadflow_size):
    if(fid not in deadflow_size):
        return False
    dflow_size = deadflow_size[fid]
    for dfs in dflow_size:
        if(dfs == totalrx):
            #print("%d is in deadflow list of fid %d" %(totalrx, fid))
            return True
    return False

def get_ideal_rates(enum):
    
    f = open(sys.argv[1]+".out", "r")
    dest_rates = {}
    time_series = {}
    for line in f:
        elems = (line.rstrip()).split(" ")
        if(elems[0] == "DestRate"):
            epoch_num = int(elems[7])
            flow_id = int(elems[2])
            time = float(elems[3])
            rate = float(elems[4])

            if(epoch_num < enum):
                continue
            elif(epoch_num > enum):
                break

            # this is the epoch we want - store each flow's rates in lists
            if(flow_id not in dest_rates):
                dest_rates[flow_id] = []
                time_series[flow_id] = []
            (dest_rates[flow_id]).append(rate)
#            (time_series[flow_id]).append(time)

    # collected all dest_rates - now check when all of them converged
    ideal_rate = {}
    for flow_id in dest_rates:
        sts = dest_rates[flow_id]
        stable_start = int(len(sts)/2.0)
        stable_end = int(len(sts) - len(sts)/4.0)
        mean = np.average(sts[stable_start:stable_end])
        ideal_rate[flow_id] = mean*1000000.0
    print("calculated ideal rates for epoch %d" %enum)
    for key in ideal_rate:
        print("flowid %d ideal rate %f" %(key, ideal_rate[key]))
    return ideal_rate

def get_epoch_data(epoch_num, ideal_rates):
    total_sent = {}
    ideal_sent = {}
    alg_sent = {}
    flow_diff_times = {}
    flow_diffs = {}
    # get flow start times before this epoch
    f = open(sys.argv[1]+".out")
    flow_start_times = get_flow_start_times(epoch_num)
    #for key in flow_start_times:
        #print("fstarts: %d %f" %(key, flow_start_times[key]))

    for line in f:
     l1 = line.rstrip();
     xy = l1.split(' ');

     if(xy[0] == "sinkdata"):
      time = float(xy[1])
      fid = int(xy[3])
      totalRx = float(xy[5])
      epoch = int(xy[7])
#     ideal_rate = float(xy[9])*1000000.0


      if(epoch < epoch_num-1):
          continue
      if(epoch > epoch_num-1):
          break
      #print ("ideal_rate of flow %d is %f" %(fid, ideal_rate))

      #print("considering flow %d epoch %d" %(fid, epoch))
      if fid in flow_start_times and flow_start_times[fid] > 0.0:
          total_sent[fid] = totalRx;
      else:
          total_sent[fid] = 0.0;
      ideal_sent[fid] = 0.0;

    f.close()

    print("list of flows who are not stopped and have sent something")
    for fid in total_sent:
        print("flow %d total_sent %f" %(fid, total_sent[fid]))

    f = open(sys.argv[1]+".out")
   # Now we know where all flows started 
    for line in f:
     l1 = line.rstrip();
     xy = l1.split(' ');

     if(xy[0] == "sinkdata"):
      time = float(xy[1])
      fid = int(xy[3])
      totalRx = float(xy[5])
      epoch = int(xy[7])

      if(epoch < epoch_num):
          continue
      if(epoch > epoch_num):
          break

      ideal_rate = ideal_rates[fid]
      if(ideal_rate == 0):
          continue
      if (fid not in ideal_sent):
          ideal_sent[fid] = 0.0
      if (fid not in total_sent):
          total_sent[fid] = 0.0
      ideal_sent[fid] += ideal_rate * sample_time #(time - flow_start_times[fid])
      alg_sent[fid] = totalRx - total_sent[fid];

      print("time %f fid %d alg_sent %f ideal_sent %f" %(time, fid, alg_sent[fid], ideal_sent[fid]))

      diff = 0.0
      if(ideal_sent[fid] > 0.0):
        diff = (ideal_sent[fid] - alg_sent[fid])/ideal_sent[fid]

      if(fid not in flow_diffs):
        flow_diffs[fid] = []
        flow_diff_times[fid] = []
      flow_diffs[fid].append(diff)
      flow_diff_times[fid].append(time)

    #print ("returning....")
    #print flow_diff_times
    #print flow_diffs
    return (flow_diff_times, flow_diffs)

ninety_fifths = []
for epochs in range(1, 35):
    ideal_rates = get_ideal_rates(epochs)
    (flow_diff_times, flow_diffs) = get_epoch_data(epochs, ideal_rates)
    #(flow_diff_times, flow_diffs) = get_epoch_data(epochs)

    # when did 95% of them come down to 10% and stay there for 10 instances
    below_10_data = []
    for key in flow_diffs:
        plt.plot(flow_diff_times[key], flow_diffs[key])
        t = below_10(flow_diffs[key])
        #print("converged at %f" %t)
        below_10_data.append(t)

    ninety_fifth = np.percentile(below_10_data, 95)
    ninety_fifth = ninety_fifth * sample_time
    print("epoch %d ninety_fifth %f" %(epochs, ninety_fifth))
    ninety_fifths.append(ninety_fifth)

