#!/usr/bin/python

import numpy as np
import random
import math
import sys

window = 100
sample_time = 0.05

details = open("details", "w")

def same_ballpark(cur_value, mean):
    if(abs(cur_value-mean) < math.ceil(0.1*mean)):
        #print("value %f mean %f value-mean %f ceil %f: MATCH" %(cur_value, mean, abs(cur_value-mean), math.ceil(0.1*mean)))
        return True
    else:
        #print("value %f mean %f value-mean %f ceil %f: NOMATCH" %(cur_value, mean, abs(cur_value-mean), math.ceil(0.1*mean)))
        return False

# Declare a DCTCP session converged when you see the same throughput over a moving
# window of few time-slots
def session_converged(sts):
    start = 0
    end = window
    converged_time = len(sts)
    stable_start = int(len(sts)/2.0)
    stable_end = int(len(sts) - len(sts)/4.0)
    mean = np.average(sts[stable_start:stable_end])
    while(end < len(sts)):
        #print("CALCULATING MEAN FOR start %d end %d "%(start, end))
        index = 0
        while(same_ballpark(sts[start+index], mean) and index<window):
            index +=1
        # Broke out of while, did we match 10?
        if(index >= window):
            print("matched %d values; start of list %d end %d window %d mean %f" %(index, start, end, window, mean))
            converged_time = start
            return (converged_time, mean)
        else:
            print("did not %d values; start of list %d end %d window %d mean %f" %(index, start, end, window, mean))
#            start += 1
            start = start+index+1
            end = start+window

    return (converged_time, mean)


def get_epoch_converged(fname, enum):
    f = open(fname, "r")
    dest_rates = {}
    time_series = {}
    tconverge = {}
    tconverge_times = []
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

    for flow_id in dest_rates:
#        start_time = (time_series[flow_id])[0]
       # print("trying to find time it took for flow %d to converge" %flow_id)
        (time_converge, mean) = session_converged(dest_rates[flow_id])
        print("optimal_value: epoch %d flow %d mean %d" %(enum, flow_id, mean))
        tconverge[flow_id] = time_converge*sample_time
        tconverge_times.append(time_converge*sample_time)

    sorted_list = sorted(tconverge_times)
    for flowid in tconverge:
        details.write("%d %d %f\n" %(enum, flowid, tconverge[flowid]))
    print("epoch %d max-time %f ms mean %f ms 95th %f ms total_flows %d" %(enum, np.max(sorted_list), np.percentile(sorted_list, 50), np.percentile(sorted_list, 95), len(tconverge_times)))

   # print("all convergence times")
   # for key in tconverge:
   #     print("%d %f" %(key, tconverge[key]))

for e in range(1, 100):
    get_epoch_converged(sys.argv[1], e)
