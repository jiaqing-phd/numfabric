# get average load of DCTCP:
# 0     1 0
# 10000 2 0.15
# 20000 3 0.2

f = 'DCTCP_CDF'
datasize_indx = 0
line_num_indx = 1
prob_indx = 2


MB_SIZE = 1000000
# 1. get the mean between any two entries, get the prob between any two entries
# get the point estimate of load, sum it to get total load

mean_prob_vec = []
mean_size_vec = []

with open(f, 'r') as f1:
    for line in f1:
        a = line.split()
        size = float(a[datasize_indx])
        prob = float(a[prob_indx])
        line_num = int(a[line_num_indx])

        if(line_num == 1):
            prev_prob = prob
            prev_size = size
            prev_line = line_num
        else:
            mean_prob = (prob  - prev_prob)
            mean_size = (size + prev_size)/2
           
            
            prev_prob = prob
            prev_size = size
            prev_line = line_num
            
            mean_prob_vec.append(mean_prob)
            mean_size_vec.append(mean_size)

            print line_num, mean_size, mean_prob, prev_prob, prev_size

    overall_DCTCP_mean = 0
    for mean_prob, mean_size in zip(mean_prob_vec, mean_size_vec):
        overall_DCTCP_mean += mean_prob * mean_size
    
    print(mean_prob_vec, mean_size_vec, overall_DCTCP_mean/MB_SIZE)   

# got mean size of DCTCP
 
