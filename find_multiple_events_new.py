#!/usr/bin/python
import sys
import os
import numpy as np
import mpsolver_convergence_times as solver
#['flow_start', '1', 'start_time', '1000000000', 'flow_size', '0', '5', '23', '1', '12440']

sim_max_time = 3.0
flow_start = "flow_start"
#log_file = sys.argv[1]
fid_index = 1
src_index = 6
dst_index = 7
fstart_index = 3
fsize_index = 5
weight_index = 8
ecmp_hash_index = 9
event_epoch = 0.05

num_flow_index = 1
num_port_index = 1

numports = 0
numflows = 0
ONEMILLION = 1000000
ONEBILLION = 1000000000.0

enough_good = 30
capacity = 10000
iter_value = 0.0005

averaged = {}


def close_enough(rate1, rate2):
    diff = (rate1 - rate2) / rate2
    if(abs(diff) < (0.2)):
        return True
    return False


def find_converge_time(ret_rates, fname, start_time, stop_time, g):
    print(
        "looking for an convergence between %f and %f in file %s" %
        (start_time, stop_time, fname))
    print("ret_rates are ")
    print(ret_rates)
    f1 = open(fname, "r")
    converged_time = {}
    times = {}
    good = {}
    flow_converged = {}

    for line in f1:
        l1 = line.rstrip()
        elems = l1.split(' ')
        if(elems[0] == "DestRate"):
            flowid = int(elems[2])
            time = float(elems[3])
            rate = float(elems[4]) / capacity

            if(time < start_time or time > stop_time):
                continue
            if(flowid in flow_converged and flow_converged[flowid]):
                continue
            if(flowid not in flow_converged):
                flow_converged[flowid] = False
                good[flowid] = 0

            if(flowid in averaged):
                averaged[flowid] = g * rate + (1 - g) * averaged[flowid]
            else:
                averaged[flowid] = rate
            times[flowid] = time
            # if(flowid in ret_rates and flowid in averaged):
            #print("flowid %d ret_rates %f rate %f time %f" %(flowid, ret_rates[flowid],rate, time))

            if((flowid in ret_rates) and (close_enough(rate, ret_rates[flowid])) and (flow_converged[flowid] == False)):
                #        print("%f checking if %f is closeeniugh to %f for flow %d" %(time, rate,ret_rates[flowid],flowid))
                good[flowid] += 1
            else:
                good[flowid] = 0
            # print(good[flowid])
            if(good[flowid] == enough_good):
                converged_time[flowid] = time - \
                    (enough_good - 1) * iter_value - start_time
                flow_converged[flowid] = True
                #print("converged point for flow %d %f optimal rate %f at %f time_to_converge %f %f %f %f"%(flowid, averaged[flowid], ret_rates[flowid], times[flowid],converged_time[flowid],iter_value,start_time,time))
    return(converged_time, flow_converged)


def get_optimal_rates(log_file, method, alpha, g):

    fh = open(log_file, "r")
    sim = solver.Simulation()
    curr_time = 1000000000.0
    num_events_parsed = 0
    max_time = (sim_max_time - .001) * ONEBILLION
    conv_time = []

    for line in fh:
        l1 = line.rstrip()
        elems = l1.split(' ')
        if(elems[0] == "topo_info"):
            numleaf = int(elems[1])
            numspines = int(elems[2])
            numPortsPerLeaf = int(elems[3])
            numports = numleaf * numPortsPerLeaf
            # edgeCapacity=2 #int(elems[4])
            # fabricCapacity=2#int(elems[5])
            edgeCapacity = int(elems[4])
            fabricCapacity = int(elems[5])
            sim.init_custom(
                numports,
                method,
                numleaf,
                numPortsPerLeaf,
                numspines,
                edgeCapacity,
                fabricCapacity)

        if((elems[0] == "flow_start") or (elems[0] == "flow_stop")):

            if(elems[0] == "flow_start"):
                flow_arrival = float(elems[fstart_index])
            if(elems[0] == "flow_stop"):
                flow_arrival = float(elems[5])
            if(flow_arrival > max_time):
                break
            if(flow_arrival > curr_time):
                event_time = curr_time / 1000000000.0
                next_event_time = event_time + event_epoch
                opt_rates = sim.startSim()  # these are optimal rates
                # print(opt_rates)
                print(" length %d time %f" % (len(opt_rates), event_time))
                (converge_times, con_flows) = find_converge_time(
                    opt_rates, log_file, event_time, next_event_time, g)

                con = 1
                max_conv = 0.0
                print("##########################################")
                for key in opt_rates:
                    if(key not in converge_times):
                        print(
                            "converge_times %f - not found flowid %d time %f" %
                            (event_epoch, key, event_time))
                        max_conv = 0.0
                        con = 0

                for key in converge_times:
                    print("converge_times: %f %d" % (converge_times[key], key))
                    if(converge_times[key] > max_conv and converge_times[key] != event_epoch):
                        max_conv = converge_times[key]
                if(con == 1):
                    conv_time.append(max_conv)
                    print("converge_times_maximum %f" % max_conv)
                print("##########################################")
                curr_time = flow_arrival

            if(elems[0] == "flow_start"):
                # new flow, we need to insert into our matrix
                #print("parsing line")
                print(elems)
                flow_id = int(elems[fid_index])
                src_id = int(elems[src_index])
                dst_id = int(elems[dst_index])
                flow_size = int(elems[fsize_index])
                if(flow_size == 0):
                    flow_size = float("inf")  # 25000000000
                flow_arrival = float(elems[fstart_index])
                weight = float(elems[weight_index])
                ecmp_hash = int(elems[ecmp_hash_index])
                sim.add_event_list(
                    flow_id,
                    flow_size,
                    flow_arrival,
                    src_id,
                    dst_id,
                    weight,
                    ecmp_hash,
                    1)

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
                sim.add_event_list(
                    flow_id,
                    flow_size,
                    flow_arrival,
                    src_id,
                    dst_id,
                    weight,
                    ecmp_hash,
                    2)
    return conv_time

get_optimal_rates(sys.argv[1], sys.argv[2], 1.0, 0.0)
