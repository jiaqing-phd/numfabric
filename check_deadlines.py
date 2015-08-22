#!/usr/bin/python
#calculate finishing times of the flows

import os
import sys
import numpy as np
import itertools

num_args = 3

if(len(sys.argv[1:]) < num_args):
    print "usage <file> <prefix> <output_dir>"

logs = open(sys.argv[1], "r")
prefix = sys.argv[2]
output_dir = sys.argv[3]
load = sys.argv[4]
estimated_load = sys.argv[5]

# data: mean RV_delta, fraction of flows with deadline

# get map from flow_id to actual deadline
# map from flow_id to stop_time

# how many deadlines are met

# for known_flows, unknown_flows
# LOAD, C_EST, RV_DELTA_MEAN, FRAC_FLOWS_DEADLINE, % DEADLINES_MET

# DEADLINE PORTION
######################
# TRUE DEADLINE
# created_deadline TRUE flow_num 15 duration 0.0132293 min_transit_time 0.000103348 flow_size 98180.5 effective_rate 1.1875e+09 link_rate 1e+10 best_transmit_time 8.26783e-05 RV_value 0.0131466 random_deadline 0.0132293 flow_start 1.09931 total_deadline 1.11254

# FALSE DEADLINE
# created_deadline FALSE flow_num 18 rand_num5 decision_frac 0.5 fraction_flows_deadline 0.5

# need to assert that best = size/effective_rate, RV + best, 

str_verbose = '.'.join(['verbose', 'deadline', str(load), str(estimated_load), str(prefix)])
str_deadline = '.'.join(['mean', 'deadline', str(load), str(estimated_load), str(prefix)])

verbose_out_file = output_dir + '/' + str_verbose
mean_out_file = output_dir + '/' + str_deadline

fstarts = {}
fstops = {}
fsizes={}
fknowns = {}

findex=1
fstartindex=3
fsizeindex=5
fknownindex=10

deadline_findex = 3
deadline_RV_value_index = 19
deadline_total_deadline_index = 23

# DEADLINE VALUES
RV_means = []

# map from flowID to deadline
# 'NA' for flowID with no deadlines
flowID_to_deadline = {}

flows_with_deadline = 0

for line in logs:
    l1 = line.rstrip()
    elems = l1.split(' ')
    # each time a flow begins
    if(elems[0] == "flow_start"):
        fid = int(elems[findex])
        fstart =  float(elems[fstartindex])
        fsize = float(elems[fsizeindex])
        fknown = int(elems[fknownindex])

        fstarts[fid] = fstart
        fsizes[fid] = fsize
        fknowns[fid] = fknown

    # each time a flow stops, NOT all flows will stop in sim time
    if(elems[0] == "flow_stop"):
        fid = int(elems[findex])
        fstop =  float(elems[fstartindex])
        fsize = float(elems[fsizeindex])
        fstops[fid] = fstop

    # each time a deadline is generated	
    if(elems[0] == 'created_deadline' and elems[1] == 'TRUE'):
        flows_with_deadline +=1
        fid = int(elems[deadline_findex])
        RV_value = float(elems[deadline_RV_value_index])
        total_deadline = float(elems[deadline_total_deadline_index])

        # populate map from flow_id to total_deadline
        RV_means.append(RV_value)
        # map from flowID to deadline
        # 'NA' for flowID with no deadlines
        flowID_to_deadline[fid] = total_deadline

# how many flows met deadlines

deadlines_met = 0
total_flows_with_deadline = 0

with open(verbose_out_file, 'w') as f:
   
    header_line = '\t'.join(['prefix', 'flow_id', 'flow_stop_time', 'local_deadline', 'deadline_met_boolean'])
    f.write(header_line + '\n')

    for flw_id,local_deadline in flowID_to_deadline.iteritems():

        if(flw_id in fstops.keys()):
            flw_stop_time = fstops[flw_id]

            nanosecond = 1e+9
            scaled_stop_time = flw_stop_time/nanosecond
            if(local_deadline != scaled_stop_time):

                if(local_deadline >= scaled_stop_time):
                    met = 1
                    deadlines_met +=1
                else:
                    met = 0    

                total_flows_with_deadline += 1

                # print("flow_id %s, flow_stop_time %s, local_deadline %s, met %s" % (flw_id, scaled_stop_time, local_deadline, met))
                out_line = '\t'.join([prefix, str(flw_id), str(scaled_stop_time), str(local_deadline), str(met)])
                f.write(out_line + '\n')

        else:
            # flow has not stopped yet, check if it has a deadline
            total_flows_with_deadline +=1 


with open(mean_out_file, 'w') as f:

    RV_mean_empirical = np.mean(RV_means)
    num_total_flows = len(fsizes.keys())

    fraction_flows_deadline = float(flows_with_deadline)/float(num_total_flows)
    fraction_met_deadline = float(deadlines_met)/float(flows_with_deadline)

    # print(RV_mean_empirical, fraction_flows_deadline, fraction_met_deadline, deadlines_met, flows_with_deadline, num_total_flows, total_flows_with_deadline)
    
    header_line = '\t'.join(['prefix', 'load', 'estimated_load', 'RV_mean_empirical', 'fraction_flows_deadline', 'fraction_met_deadline', 'total_deadlines_met', 'total_flows_deadline', 'num_total_flows', 'total_flows_with_deadline'])
    f.write(header_line + '\n')

    out_line = '\t'.join([prefix, load, estimated_load, str(RV_mean_empirical), str(fraction_flows_deadline), str(fraction_met_deadline), str(deadlines_met), str(flows_with_deadline), str(num_total_flows), str(total_flows_with_deadline)])
    
    f.write(out_line + '\n')
