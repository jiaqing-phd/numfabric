#!/usr/bin/python
import sys

enough_good = 100
iter_value=0.00001

def close_enough(rate1, rate2):
  diff = rate1 - rate2
  if(abs(diff) < (0.1*rate2)):
    return True
  return False

def find_converge_time(ret_rates, fname, start_time, stop_time):
  f1 = open(fname, "r")
  converged_time = {}
  rates = {}
  times= {}
  good = {}
  flow_converged = {}

  for line in f1:
    l1 = line.rstrip()
    elems = l1.split(' ')
    if(elems[0] == "DestRate"):
      flowid = int(elems[2])
      time = float(elems[3])
      rate = float(elems[5])

      if(time < start_time):
        continue
      if(time > stop_time):
        break
      if(flowid not in rates):
        flow_converged[flowid] = False
        good[flowid] = 0

      rates[flowid] = rate
      times[flowid] = time
     
#      if(flowid == 2 and time > 1.8):
#        print("time %f cur rate %f flowid %d ret_rates %f" %(time, rates[flowid], flowid, ret_rates[flowid])) 
      if(flowid in ret_rates):
        if(not(close_enough(rates[flowid], ret_rates[flowid])) and (flow_converged[flowid] == False)):
          good[flowid] = 0
        
        if((close_enough(rates[flowid], ret_rates[flowid])) and (flow_converged[flowid] == False)):
          good[flowid] += 1
          
          if(good[flowid] == enough_good):
            converged_time[flowid] = time
            flow_converged[flowid] = True
  f1.close()
  return converged_time

def get_optimal_rates(log_file, start_time):
  f1 = open(log_file, "r")
  trueRate = {}

  for line in f1:
    l1 = line.rstrip()
    elems = l1.split(' ')
    if(elems[0] == "TrueRate"):
      t = float(elems[1])
      if((t - start_time) < 0.000001):
        fid = int(elems[2])
        rate = float(elems[3])/1000000.0
        trueRate[fid] = rate
  f1.close()
  return trueRate
    

log_file = sys.argv[1]
time_start = 1.0

while(time_start < 2.01):
  ret_rates = get_optimal_rates(log_file, time_start)
  #for key in ret_rates:
  #  print("%d %f" %(key,ret_rates[key]))
  converged_times = find_converge_time(ret_rates, log_file, time_start, time_start+0.1)
#  print("converged times is %f+... "%time_start)
  print ("time %f" %time_start)
  for key in converged_times:
    print("flow %d converged after %f seconds" %(key, (converged_times[key] - (enough_good*iter_value) - time_start)))
  time_start = time_start+0.2

