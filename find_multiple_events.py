#!/usr/bin/python
import sys
import os
import scipy.io as sio
import pickle
import numpy as np
import mpsolver_convergence_times as solver
#['flow_start', '1', 'start_time', '1000000000', 'flow_size', '0', '5', '23', '1', '12440']

flow_start = "flow_start"
#log_file = sys.argv[1]
fid_index = 1
src_index = 6
dst_index = 7
fstart_index = 3
fsize_index = 5
weight_index=8
ecmp_hash_index=9
event_epoch=0.1

max_sim_time=1.199
num_flow_index = 1
num_port_index = 1

numports = 0
numflows = 0
ONEMILLION = 1000000

enough_good = 30
capacity=10000
iter_value=0.0001

averaged = {}
OptRates={}


def close_enough(rate1, rate2):
  diff = (rate1 - rate2)/rate2
  if(abs(diff) < (0.1)):
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
#        print("%f checking if %f is closeeniugh to %f for flow %d" %(time, rate,ret_rates[flowid],flowid))
        good[flowid] += 1
      else:
        good[flowid] = 0
      #print(good[flowid])
      if(good[flowid] == enough_good):
        converged_time[flowid] = time - (enough_good-1)*iter_value - start_time
        flow_converged[flowid] = True
        #print("converged point for flow %d %f optimal rate %f at %f time_to_converge %f %f %f %f"%(flowid, averaged[flowid], ret_rates[flowid], times[flowid],converged_time[flowid],iter_value,start_time,time))
  return(converged_time, flow_converged)

def get_optimal_rates(log_file, method, alpha, g, num_events): 

        fh = open(log_file, "r")
        sim = solver.Simulation()

        num_events_parsed=0
        print("num_events %d" %num_events)
        num_epoch = 1

        output = open("dummy", "w")
        for line in fh:
          l1 = line.rstrip();
          elems = l1.split(' ')
    	  if(elems[0] == "topo_info"):
      		  numleaf = int(elems[1])
      		  numspines = int(elems[2])
      		  numPortsPerLeaf = int(elems[3])
      		  numports=numleaf*numPortsPerLeaf
      		  #edgeCapacity=2 #int(elems[4])
      		  #fabricCapacity=2#int(elems[5])
      		  edgeCapacity=int(elems[4])
      		  fabricCapacity=float(elems[5])
      		  sim.init_custom(numports, method, numleaf,numPortsPerLeaf, numspines, edgeCapacity,fabricCapacity,alpha)
		  opt_file_name="opt_rates_"+str(edgeCapacity)+"_"+str(fabricCapacity)+"_"+str(alpha)+"test"
		  output = open(opt_file_name, "w")


          if((elems[0] == "flow_start") or (elems[0] == "flow_stop")):
            num_events_parsed += 1
            if(elems[0] == "flow_start"):
                # new flow, we need to insert into our matrix 
                #print("parsing line")
                print(elems)
                flow_id = int(elems[fid_index])
                src_id = int(elems[src_index]) 
                dst_id = int(elems[dst_index])
                flow_size = int(elems[fsize_index])
                if(flow_size == 0):
          		    flow_size = float("inf") #25000000000
                flow_arrival = float(elems[fstart_index])
                weight = float(elems[weight_index])
                ecmp_hash = int(elems[ecmp_hash_index])
                sim.add_event_list(flow_id, flow_size, flow_arrival, src_id, dst_id, weight, ecmp_hash, 1)

            if(elems[0] == "flow_stop"): 
                #print("flow_stop event ")
                print(elems)
                flow_id = int(elems[1])
                src_id = int(elems[2])
                dst_id = int(elems[3])
                flow_size = 0
                flow_arrival = float(elems[5])
                weight = 1
                ecmp_hash = 0
                sim.add_event_list(flow_id, flow_size, flow_arrival, src_id, dst_id, weight, ecmp_hash, 2)

            if(num_events_parsed == num_events):
                num_events_parsed=0
                event_time = flow_arrival/1000000000.0;
                if (event_time > max_sim_time):
                    break
                next_event_time = event_time+ event_epoch;
                
                (opt_rates,realId) = sim.startSim() #these are optimal rates
                print("opt_rates at time %d" %num_epoch)
                print(opt_rates)
                #OptRates[event_time]=opt_rates
                for key in opt_rates:
                  output.write("%d %d %f\n" %(num_epoch, key, opt_rates[key]))
                num_epoch = num_epoch+1
                #FlowId[event_time]=realId  

		#(converge_times, con_flows) = plot_converge_time(opt_rates, log_file, event_time, next_event_time, g) 
                """ 
                (converge_times, con_flows) = find_converge_time(opt_rates, log_file, event_time, next_event_time, g) 
                con = 1
                max_conv=0.0
                print("##########################################")
                for key in opt_rates:
                    if(key not in converge_times):
                        print("converge_times %f - not found flowid %d time %f" %(event_epoch,key,event_time))
                        max_conv=0.0
                        con = 0

                for key in converge_times:
                    print("converge_times: %f %d" %(converge_times[key], key))
                    if(converge_times[key] > max_conv and converge_times[key] != event_epoch):
                        max_conv=converge_times[key]
                if(con == 1):
                  print("converge_times_maximum %f" %max_conv)
		  
                print("##########################################")
                """
        #savefile= open((log_file + ".npz"), 'wb')
        #print(" length %d time %f" %( len(opt_rates) ,event_time))
        #pickle.dump(OptRates,savefile)  
        #output.close()
## log_file, 
get_optimal_rates(sys.argv[1], "mp", sys.argv[2], 0.0, int(sys.argv[3]))
#def get_optimal_rates(log_file, method, alpha, g, num_events): 
