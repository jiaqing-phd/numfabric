f = 'heavy.DCTCP_CDF.txt'
datasize_indx = 0
prob_indx = 2

size_cutoff = 4e7
f_out = 'heavy.scaled.DCTCP_CDF.size.' + str(size_cutoff) + '.txt'

f2 = open(f_out, 'w')

scale_factor = 1.0

with open(f, 'r') as f1:
    for line in f1:
        a = line.split()
        size = float(a[datasize_indx])
        prob = float(a[prob_indx])

        if(size <= size_cutoff):
            scale_factor = max(prob,.00001)  

with open(f, 'r') as f1:
    index = 0
    for line in f1:
        index+=1
        a = line.split()
        size = float(a[datasize_indx])
        prob = float(a[prob_indx])

        if(size <= size_cutoff):
            scaled_prob = float(prob)/float(scale_factor)
            print(size, prob, scale_factor, scaled_prob)
            # 5 50000 5 0.4
            # 6 80000 6 0.53
            # 7 200000 7 0.6
            out_line = '\t'.join([str(size),str(index),str(scaled_prob)]) + '\n'
            f2.write(out_line)


with open(f, 'r') as f1:
    index = 0
    for line in f1:
        index+=1
        a = line.split()
        size = float(a[datasize_indx])
        prob = float(a[prob_indx])

        if(size >= size_cutoff):
            scaled_prob = float(prob)/float(scale_factor)
            print(size, prob, scale_factor, scaled_prob)
            # 5 50000 5 0.4
            # 6 80000 6 0.53
            # 7 200000 7 0.6
            out_line = '\t'.join([str(size),str(index),str(scaled_prob)]) + '\n'
            f2.write(out_line)

f_heavy_name = 'scaled.heavy.DCTCP_CDF.' + str(size_cutoff) + '.txt' 
f_heavy = open(f_heavy_name, 'w')

with open(f, 'r') as f1:
    index = 0
    for line in f1:
        index+=1
        a = line.split()
        size = float(a[datasize_indx])
        prob = float(a[prob_indx])

        if(size >= size_cutoff):
            scaled_prob = float(prob) - float(scale_factor)
            print(size, prob, scale_factor, scaled_prob)
            out_line = '\t'.join([str(size),str(index),str(scaled_prob)]) + '\n'
            f_heavy.write(out_line)

f_heavy.close()           
