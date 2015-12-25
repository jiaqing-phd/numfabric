#!/usr/bin/python
import sys
import os
import numpy as np
import mpsolverdynamic_convergence_times as solver

flow_start = "flow_start"
#log_file = sys.argv[1]
fid_index = 1
src_index = 2
dst_index = 3
fstart_index = 5
fsize_index = 6

num_flow_index = 1
num_port_index = 1

numports = 0
numflows = 0
ONEMILLION = 1000000

enough_good = 50
capacity=10000
iter_value=0.00001

averaged = {}

def close_enough(rate1, rate2):
  diff = (rate1 - rate2)/rate2
  print("diff is %f" %diff)
  if(abs(diff) < (0.05)):
    return True
  return False

def find_converge_time(ret_rates, fname, start_time, stop_time, g):
  print("looking for an convergence between %f and %f in file %s" %(start_time, stop_time, fname))
  print("ret_rates are ")
  print(ret_rates)
  f1 = open(fname, "r")
  converged_time = {}
  times= {}
  good = {}
  flow_converged = {}

  for line in f1:
    l1 = line.rstrip()
    elems = l1.split(' ')
    if(elems[0] == "DestRate"):
      flowid = int(elems[2])
      time = float(elems[3])
      rate = float(elems[4])/capacity

      if(time < start_time or time > stop_time):
        continue
      if(flowid in flow_converged and flow_converged[flowid] == True):
        continue
      if(flowid not in flow_converged):
        flow_converged[flowid] = False
        good[flowid] = 0


      if(flowid in averaged):
        averaged[flowid] = g*rate + (1-g)*averaged[flowid]
      else:
       averaged[flowid] = rate
      times[flowid] = time 
      #if(flowid in ret_rates and flowid in averaged): 
        #print("flowid %d ret_rates %f rate %f time %f" %(flowid, ret_rates[flowid],rate, time)) 
    
      if((flowid in ret_rates) and (close_enough(rate, ret_rates[flowid])) and (flow_converged[flowid] == False)):
        print("%f checking if %f is closeeniugh to %f for flow %d" %(time, rate,ret_rates[flowid],flowid))
        good[flowid] += 1
      else:
        good[flowid] = 0
      #print(good[flowid])
      if(good[flowid] == enough_good):
        converged_time[flowid] = time - (enough_good-1)*iter_value - start_time
        flow_converged[flowid] = True
        print("converged point for flow %d %f optimal rate %f at %f time_to_converge %f %f %f %f"%(flowid, averaged[flowid], ret_rates[flowid], times[flowid],converged_time[flowid],iter_value,start_time,time))
  return(converged_time, flow_converged)

def getclass(self, srcid, dstid):
  if(srcid == 0 and dstid == 2):
    return 1
  if(srcid == 0 and dstid == 3):
    return 2
  if(srcid == 1 and dstid == 2):
    return 3
  print("unknown src %d dst %d pair" %(srcid, dstid))

def get_optimal_rates(log_file, method, alpha, g):

        fh = open(log_file, "r")
        numports = 14
        sim = solver.Simulation()
        sim.init_custom(numports, method, alpha)

        w = [1.0,1.0,2.0,6.0,3.0,1.0]
        for line in fh:
          l1 = line.rstrip();
          elems = l1.split(' ')
          if(len(elems) > 1):
            if(elems[0] == "num_flows"):
              numflows = int(elems[num_flow_index])
              # Allocate matrix now:
              f_matrix = np.zeros((numflows, numports))
              w = np.ones((numflows, 1)) # tbd
              c = np.ones((numports, 1))
              print("Allocated a matrix with %d rows and %d columns" %(numflows, numports))
            #if(elems[0] == "num_ports"):
            #  numports = int(elems[num_port_index])
            #  sim.init_custom(numports, method)
            if((elems[0] == "flow_start") or (elems[0] == "flow_stop")):
              # new flow, we need to insert into our matrix 
              print("parsing line")
              print(elems)
              flow_id = int(elems[fid_index])
              src_id = int(elems[src_index]) 
              dst_id = int(elems[dst_index])
              flow_size = 21.0 #int(elems[fsize_index])
              flow_arrival = float(elems[fstart_index])
              weight = float(elems[10])

              if(elems[0] == "flow_start"):
                print("flow_start event ")
                sim.add_event_list(flow_id, flow_size, flow_arrival, src_id, dst_id, weight, 1)
                #print("ADDING FLOW %d "%flow_id)
              if(elems[0] == "flow_stop"):
                sim.add_event_list(flow_id, flow_size, flow_arrival, src_id, dst_id, weight, 2)

              event_time = flow_arrival/1000000000.0;
              next_event_time = event_time+ 0.1
              opt_rates = sim.startSim() #these are optimal rates
              print("found optimal rates")
              print(opt_rates) 
              (converge_times, con_flows) = find_converge_time(opt_rates, log_file, event_time, next_event_time, g) 
              con = 1
              max_conv=0.0
              print("##########################################")
              for key in opt_rates:
                if(key not in converge_times):
                  print("converge_times 0.05 - not found flowid %d" %key)
                  max_conv=0.0
                  con = 0

              for key in converge_times:
                 print("converge_times: %f %d" %(converge_times[key], key))
                 if(converge_times[key] > max_conv and converge_times[key] != 0.05):
                    max_conv=converge_times[key]
              if(con == 1):
                  print("converge_times_maximum %f" %max_conv)
              print("##########################################")
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
        #opt_rates = sim.startSim() #these are optimal rates
        #min_f=1
        #min_r = 1.0
        #rate_sum = 0
        #for f in opt_rates:
          #rate_sum+=opt_rates[f]
          #if(opt_rates[f] < min_r):
            #min_r = opt_rates[f]
            #min_f = f
        #print("alpha  %f min_r %f sum_rate %f min_flow=%d" %(alpha,min_r,rate_sum, min_f))
            

get_optimal_rates(sys.argv[1], sys.argv[2], float(sys.argv[3]), 0.0)
