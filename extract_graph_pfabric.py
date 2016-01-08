#!/usr/bin/python
import sys
import os
import numpy as np
import mp_solver_pfabric_link as solver
import math

flow_start = "flow_start"
log_file = sys.argv[1]
method = sys.argv[2]
fid_index = 1
src_index = 6
dst_index = 7
fstart_index = 3
ecmp_hash_index = 9
weight_index = 8
fsize_index = 5

num_flow_index = 1
num_port_index = 1

numports = 0
numflows = 0

data_size =1410
pkt_size = 1500

fh = open(log_file, "r")

sim = solver.Simulation()

for line in fh:
  l1 = line.rstrip();
  elems = l1.split(' ')
  if(len(elems) > 1):
    if(elems[0] == "topo_info"):
      numleaf = int(elems[1])
      numspines = int(elems[2])
      numPortsPerLeaf = int(elems[3])
      numports=numleaf*numPortsPerLeaf
      #edgeCapacity=2 #int(elems[4])
      #fabricCapacity=2#int(elems[5])
      edgeCapacity=int(elems[4])
      fabricCapacity=int(elems[5])
      sim.init_custom(numports, method, numleaf,numPortsPerLeaf, numspines, edgeCapacity,fabricCapacity )
    if(elems[0] == "flow_start"):
      # new flow, we need to insert into our matrix
      flow_id = int(elems[fid_index])
      src_id = int(elems[src_index])
      dst_id = int(elems[dst_index])
      flow_size = int(elems[fsize_index])
      # add header sizes to this flow_size
      num_packets = math.ceil(flow_size*1.0/data_size)
      headers_total = num_packets * (pkt_size - data_size)
#      print("flow_size %d num_packets %d headers_total %d" %(flow_size,num_packets,headers_total))
      flow_size = flow_size + headers_total

      if(flow_size == 0):
          flow_size = 2500000000
      flow_arrival = float(elems[fstart_index])
      weight = float(elems[weight_index])
      ecmp_hash = int(elems[ecmp_hash_index])

      if(elems[0] == "flow_start"):
        #print("adding flow ...");
        #print(" src %d, %d, %d, %d, %d, %d, %d " %(src_id, dst_id, flow_id, flow_size, flow_arrival, weight, ecmp_hash))
        sim.add_flow_list(src_id, dst_id, flow_id, flow_size, flow_arrival, weight, ecmp_hash)
        #f_matrix[flow_id, src_id] = 1
        #f_matrix[flow_id, dst_id] = 1
        #new_row = np.ones((numports, 0))
        #new_row[src_id] = 1
        #new_row[dst_id] = 1
        #add_row(new_row, flow_id, flow_size)
        #print("flow added (%d %d %d) A=" %(flow_id, src_id, dst_id))
        #print f_matrix
        #solver.main_solver(f_matrix, w, c, numports, numflows)
      #else:
        #f_matrix[flow_id, src_id] = 0
        #f_matrix[flow_id, dst_id] = 0
        #print("flow removed (%d %d %d) A=" %(flow_id, src_id, dst_id))
        #print f_matrix
        #solver.main_solver(f_matrix, w, c, numports/2.0, numflows)

#After everything...

sim.startSim()


