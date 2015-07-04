#!/usr/bin/python

#calculate finishing times of the flows

import os
import sys
import numpy as np
logs = open(sys.argv[1], "r")

fstarts = {}
fstops = {}
fsizes={}

findex=1
fstartindex=3
fsizeindex=5

# SC known 0 UNKNOWN_FLOW_SIZE_CUTOFF 1e+06 flow_size 89191.7 flow_num 49
# SC known 0 UNKNOWN_FLOW_SIZE_CUTOFF 1e+06 flow_size 725860 flow_num 50

flow_known_indx = 2
flow_cutoff_indx = 4
flow_size_indx = 6
flow_num_indx = 8

total_flows = 0
total_bytes = 0
known_flows = 0
unknown_flows = 0

flow_size_dict = {}
flow_size_dict['known'] = []
flow_size_dict['unknown'] = []

for line in logs:
    l1 = line.rstrip()
    elems = l1.split(' ')

    if(elems[0] == "SC_DCTCP_DEBUG" and elems[1] == "known"):
        known_flow = int(elems[flow_known_indx])
        flow_cutoff = elems[flow_cutoff_indx]
        flow_size = float(elems[flow_size_indx])
        flow_num = elems[flow_num_indx]

        total_flows +=1

        if(known_flow == 1):
            flow_size_dict['known'].append(flow_size)
            known_flows +=1
        else:
            flow_size_dict['unknown'].append(flow_size)
            unknown_flows +=1

print("known flows %d unknown flows %d" %(known_flows,unknown_flows))
frac_unknown = float(unknown_flows)/float(total_flows)

unknown_bytes = sum(flow_size_dict['unknown']) 
total_bytes = sum(flow_size_dict['known']) + sum(flow_size_dict['unknown'])
frac_unknown_traffic_bytes = float(unknown_bytes/total_bytes)

print frac_unknown, frac_unknown_traffic_bytes
print("Fraction unknown %f , fraction unknown (portion of bytes) %f " %(frac_unknown, frac_unknown_traffic_bytes))

