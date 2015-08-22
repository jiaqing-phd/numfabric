#!/usr/bin/python

#calculate finishing times of the flows

import os
import sys
import numpy as np

logs = open(sys.argv[1], "r")
CDF_data = sys.argv[2]
load = sys.argv[3]

f = open(CDF_data, 'w')

fstarts = {}
fstops = {}
fsizes={}

findex=1
fstartindex=3
fsizeindex=5

# SC known 0 UNKNOWN_FLOW_SIZE_CUTOFF 1e+06 flow_size 89191.7 flow_num 49
# SC known 0 UNKNOWN_FLOW_SIZE_CUTOFF 1e+06 flow_size 725860 flow_num 50

# alternate version

#flow_known_indx = 2
#flow_cutoff_indx = 4
#flow_size_indx = 6
#flow_num_indx = 8

flow_size_indx=12

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

    # unknown flow between 2 and 4 starting at time 1.07673 of size 969292 flow_num 14

    if(len(elems) <=2):
        continue

    if(elems[1] == "flow" and elems[2] == "between"):
        if (elems[0] == 'known'):
            known_flow = 1
        else:
            known_flow = 0
        
        flow_size = float(elems[flow_size_indx])
        total_flows +=1

        if(known_flow == 1):
            flow_size_dict['known'].append(flow_size)
            known_flows +=1
        else:
            flow_size_dict['unknown'].append(flow_size)
            unknown_flows +=1

frac_unknown = float(unknown_flows)/float(total_flows)

unknown_bytes = sum(flow_size_dict['unknown']) 
total_bytes = sum(flow_size_dict['known']) + sum(flow_size_dict['unknown'])
frac_unknown_traffic_bytes = float(unknown_bytes/total_bytes)

print frac_unknown, frac_unknown_traffic_bytes
print("Fraction unknown %f , fraction unknown (portion of bytes) %f, known flows %s, unknown flows %f, total flows %f " %(frac_unknown, frac_unknown_traffic_bytes, known_flows, unknown_flows, total_flows))

header_line = '\t'.join(['# load, % unknown_flows, % unknown_traffic, num_unknown_flows, total_flows, unknown_bytes, total_bytes']) 
f.write(header_line + '\n')

out_line = '\t'.join([str(load), str(frac_unknown), str(frac_unknown_traffic_bytes), str(known_flows), str(unknown_flows), str(total_flows), str(unknown_bytes), str(total_bytes)]) 
f.write(out_line + '\n')

f.close()


