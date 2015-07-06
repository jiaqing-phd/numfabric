#!/usr/bin/python
#calculate finishing times of the flows

import os
import sys
import numpy as np

num_args = 3

if(len(sys.argv[1:]) < num_args):
    print "usage <file> <prefix> <output_dir>"

logs = open(sys.argv[1], "r")
prefix = sys.argv[2]
output_dir = sys.argv[3]
load = sys.argv[4]
est_load = sys.argv[5]

out_file = output_dir + '/' + 'UNKNOWN.FCT.' + prefix
verbose_out_file = output_dir + '/' + 'UNKNOWN.verbose.FCT.' + prefix
mean_out_file = output_dir + '/' +  'UNKNOWN.mean.FCT.' + prefix

known_out_file = output_dir + '/' + 'KNOWN.FCT.' + prefix
known_verbose_out_file = output_dir + '/' + 'KNOWN.verbose.FCT.' + prefix
known_mean_out_file = output_dir + '/' +  'KNOWN.mean.FCT.' + prefix

fstarts = {}
fstops = {}
fsizes={}
fknowns = {}

findex=1
fstartindex=3
fsizeindex=5
fknownindex=10

for line in logs:
    l1 = line.rstrip()
    elems = l1.split(' ')
    if(elems[0] == "flow_start"):
        fid = int(elems[findex])
        fstart =  float(elems[fstartindex])
        fsize = float(elems[fsizeindex])
        fknown = int(elems[fknownindex])

        fstarts[fid] = fstart
        fsizes[fid] = fsize
        fknowns[fid] = fknown

    if(elems[0] == "flow_stop"):
        fid = int(elems[findex])
        fstop =  float(elems[fstartindex])
        fsize = float(elems[fsizeindex])

        fstops[fid] = fstop

f_verbose = open(verbose_out_file, 'w')
f_mean = open(mean_out_file, 'w')

# if we want to plot flow inter-arrival distros
# flow_start 565 start_time 1647469736 flow_size 98417 2 5 port 242

# UNKNOWN FLOWS
##############################
# out file, very small one
with open(out_file, 'w') as f:

    ftime_vec = []
    normalized_ftime_vec = []
    size_vec = []

    for key in fstarts:
        if(key in fstops and fknowns[key] == 0):
            ftime = (fstops[key] - fstarts[key])/1000000.0
            #print("UNKNOWN Flow %d Size %d bytes finished in %fms" %(key, fsizes[key], ftime))

            x = "UNKNOWN Flow %d Size %d bytes finished in %fms" %(key, fsizes[key],
            ftime) + '\n'
            f_verbose.write(x)

            # x/s
            time_normalized = float(ftime)/float(fsizes[key])

            # ratio = time/minimum_time to finish
            out_line = '\t'.join([str(key), str(fsizes[key]), str(ftime),
            str(time_normalized)])
            f.write(out_line + '\n')

            # MEAN FILE
            ftime_vec.append(ftime)
            normalized_ftime_vec.append(time_normalized)

            size_vec.append(fsizes[key])

    mean_ftime = np.mean(ftime_vec)
    std_ftime = np.std(ftime_vec)

    mean_size = np.mean(size_vec)
    std_size = np.std(size_vec)

    mean_normalized_ftime = np.mean(normalized_ftime_vec)
    std_normalized_ftime = np.std(normalized_ftime_vec)

    mean_out_line = '\t'.join(['# UNKNOWN LOAD', 'EST_LOAD', 'MEAN FCT(ms)', 'STD FCT (ms)', 'SCALED_MEAN FCT (ms)', 'SCALED_STD FCT (ms), MEAN_SIZE, STD_SIZE'])
    f_mean.write(mean_out_line + '\n')

    mean_out_line = '\t'.join([str(load), str(est_load), str(mean_ftime), str(std_ftime), str(mean_normalized_ftime), str(std_normalized_ftime), str(mean_size), str(std_size)])
    f_mean.write(mean_out_line + '\n')

# get a distribution of flow size
f_verbose.close()
f_mean.close()



known_f_verbose = open(known_verbose_out_file, 'w')
known_f_mean = open(known_mean_out_file, 'w')

##############################
# KNOWN FLOWS
# out file, very small one
with open(known_out_file, 'w') as f:

    ftime_vec = []
    normalized_ftime_vec = []
    size_vec = []

    for key in fstarts:
        if(key in fstops and fknowns[key] == 1):
            ftime = (fstops[key] - fstarts[key])/1000000.0
            #print("KNOWN Flow %d Size %d bytes finished in %fms" %(key, fsizes[key], ftime))

            x = "KNOWN Flow %d Size %d bytes finished in %fms" %(key, fsizes[key],
            ftime) + '\n'
            known_f_verbose.write(x)

            # x/s
            time_normalized = float(ftime)/float(fsizes[key])

            # ratio = time/minimum_time to finish
            out_line = '\t'.join([str(key), str(fsizes[key]), str(ftime),
            str(time_normalized)])
            f.write(out_line + '\n')

            # MEAN FILE
            ftime_vec.append(ftime)
            normalized_ftime_vec.append(time_normalized)

            size_vec.append(fsizes[key])

    mean_ftime = np.mean(ftime_vec)
    std_ftime = np.std(ftime_vec)

    mean_size = np.mean(size_vec)
    std_size = np.std(size_vec)

    mean_normalized_ftime = np.mean(normalized_ftime_vec)
    std_normalized_ftime = np.std(normalized_ftime_vec)

    mean_out_line = '\t'.join(['#LOAD', 'EST_LOAD', 'MEAN FCT(ms)', 'STD FCT (ms)', 'SCALED_MEAN FCT (ms)', 'SCALED_STD FCT (ms), MEAN_SIZE, STD_SIZE'])
    known_f_mean.write(mean_out_line + '\n')

    mean_out_line = '\t'.join([str(load), str(est_load), str(mean_ftime), str(std_ftime), str(mean_normalized_ftime), str(std_normalized_ftime), str(mean_size), str(std_size)])
    
    known_f_mean.write(mean_out_line + '\n')

# get a distribution of flow size
known_f_verbose.close()
known_f_mean.close()
