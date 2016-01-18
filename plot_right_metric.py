#!/usr/bin/python
import numpy as np
import matplotlib.pyplot as plt
import sys
import os
import pickle
import operator

max_time=1.249

def ewma(values, g=1.0/8):
    ret = []
    prev = 0
    for v in values:
        prev = prev * (1.0 - g) + v * g
        ret.append(prev)
    return ret

def fx(numbers,func):
    if (func==1.0):
        numbers.sort()
        median = numbers[len(numbers)/2]
        return median
    if (func==2.0):
        return np.mean( numbers )
    if (func==3.0):
        return np.percentile(numbers, 95)
    if (func==4.0):
        return np.max(numbers)

def plot_time_series(sumDiffRates,fig_in,color,legend_val,folder,pre,subplot):
    sumDiffRates = sorted(sumDiffRates.items(), key=operator.itemgetter(0))
    xs = [x[0] for x in sumDiffRates]
    ys = [x[1] for x in sumDiffRates]
    
    plt.figure(fig_in)
    plt.subplot(2,1,subplot)
    plt.plot(xs, ys, color, label=str(pre + " " + legend_val))
    plt.yscale('log')

    plt.xlabel('Time in seconds')
    plt.ylabel('Rate in Mbps')
    plt.title('%s_rates' %(pre) )
    #plt.legend(loc='upper right')
    if (subplot==2):
        lgd=plt.legend(loc='upper center', bbox_to_anchor=(0.5,-0.1))
        plt.savefig('%s/%s_%s.png' %(folder,pre,"timeseries"), \
        bbox_extra_artists=(lgd,), bbox_inches='tight')
        #plt.draw()
        #plt.show()


def SecondTerm(prices, utils, eta=10.0):
    ret = []
    prev = 0
    index=0
    for v in utils:
        curr = -prev * eta * max(0,v)
        prev= prices[index]
        index+=1
        ret.append(curr)
    return ret

def main(filename , folder, base_val, color):
    base_val= 10* int(base_val)
    print(filename)
    optfile=open(filename+ ".out.npz","rb")
    f = open(filename+".out")
    pre = filename
    savefilename=filename+ "_timeseries_data.npz"

    if not os.path.exists(folder):
      os.makedirs(folder)
    xy = []
    x = {}
    y = {}
    
    rtime = []
    trate = []
    
    dtimes = {}
    drates = {}
    diffRates = {}
    diffPercentRates = {}
    dprios = {}
    total_capacity=1000000
    
    qtimes = {}
    qsizes = {}
    qprices = {}
    qrates = {}
    qutils = {}
    qminresidues = {}
    oRates = {}
    q0times = {}
    q0deadlines = {}
    q1times = {}
    q1deadlines = {}
    rincrements = {}
    timeDiffRates={}
    timeDiffPercentRates={}
    
    sumDiffRates={}
    sumPercentDiffRates={}
    optRates=pickle.load(optfile)
        
    if not os.path.isfile(savefilename):
        savefile= open(savefilename,"wb")
        for line in f:
          l1 = line.rstrip();
          xy = l1.split(' ');
        
          if(xy[0] == "DestRate"):
            flow_id=int(xy[2])
            t1 = float(xy[3])
            rate=float(xy[4])
            if (t1 > max_time):
                break
            sumDiffRates[t1]=[]
            sumPercentDiffRates[t1]=[]
            timeDiffRates[t1]=[]
            timeDiffPercentRates[t1]=[]
        
            if(flow_id not in dtimes):
              dtimes[flow_id] = []
              drates[flow_id] = []
              diffRates[flow_id]=[]
              diffPercentRates[flow_id]= []
              oRates[flow_id]=[]
            sorted_keys=np.sort(optRates.keys());
            prevkey=sorted_keys[0];
            for key in sorted_keys:
              if( key > t1 ):
                 break
              prevkey=key
            #print(" time %f and key %f " %(t1,prevkey))
            dtimes[flow_id].append(t1)
            drates[flow_id].append(rate)
            oRates[flow_id].append(10000.0 *optRates[prevkey][flow_id])
            diffRates[flow_id].append( abs(rate - 10000.0 *optRates[prevkey][flow_id]))
            diffPercentRates[flow_id].append(float( abs(rate - 10000.0* \
            optRates[prevkey][flow_id]))/(10000.0 * optRates[prevkey][flow_id]) )
        
          if(xy[0] == "QUEUESTATS"):
            qtime = float(xy[1])
            queue_id = xy[2]
            qprice = float(xy[3])
            qutil = float(xy[4])
            qmin_residue = float(xy[5])
            qsize = int(xy[6])
        
        
            if(queue_id not in qtimes):
              qtimes[queue_id] = []
              qprices[queue_id] = []
              qutils[queue_id] = []
              qminresidues[queue_id] = []
              qsizes[queue_id] = []
        
        
            qtimes[queue_id].append(qtime)
            qprices[queue_id].append(qprice)
            qutils[queue_id].append(qutil)
            qminresidues[queue_id].append(qmin_residue)
            qsizes[queue_id].append(qsize)
        
        
        colors = ['r','b','g', 'm', 'c', 'y','k','#fedcba','#abcdef','#ababab','#badaff','#deadbe','#bedead','#afafaf','#8eba42','#e5e5e5','#6d904f']
        
        
        
        i=0
        j=1;
        """
        for key in dtimes:
            plt.figure(5)
            plt.subplot(2,1,1)
            plt.plot(dtimes[key], drates[key], color=colors[i], label=str(key))
            i = (i+1)%len(colors)
            plt.plot(dtimes[key], oRates[key], color=colors[i], label=str(key))
            i = (i+1)%len(colors)                 
            plt.plot(dtimes[key], diffRates[key], color=colors[i], label=str(key))
            i = (i+1)%len(colors)   
            plt.subplot(2,1,2)
            plt.plot(dtimes[key], diffPercentRates[key], color=colors[i], label=str(key))
            #plt.draw()
            #plt.show()
            plt.savefig('%s/%s_%s.png' %(folder,(pre+str(key)),"rates"))
            plt.close()
        """
        for key in dtimes:
          iter=0
          for time in dtimes[key]:
              #print(" time %f, flowid %d diff rate %f " %(time,key,diffRates[key][iter]) )  
              timeDiffRates[time].append(diffRates[key][iter]) 
              timeDiffPercentRates[time].append(diffPercentRates[key][iter])
              iter+=1
        
        pickle.dump([timeDiffPercentRates,timeDiffRates],savefile,pickle.HIGHEST_PROTOCOL)
    else:
        [timeDiffPercentRates,timeDiffRates]=pickle.load(open(savefilename,"rb"))
    iter=0
    legend_val=['median', 'avg','_95_percentile', 'max' ]
    j=0
    fun_vec= [1.0,2.0,3.0,4.0] 
    
    for numfunc in fun_vec:
    
        for time in timeDiffRates:
            #print(" time %f, flowid %d diff rate %f " %(time,key,diffRates[key][iter]) )  
            sumDiffRates[time]= fx(timeDiffRates[time], numfunc) 
            sumPercentDiffRates[time]= fx(timeDiffPercentRates[time],numfunc)
        plot_time_series(sumDiffRates, base_val+ int(numfunc),color,\
         legend_val[iter],folder,pre + legend_val[iter] + "_diffrates ", 1)   
        j+=1
        plot_time_series(sumPercentDiffRates, base_val + int(numfunc),\
        color, legend_val[iter],folder,pre + legend_val[iter] + "_percent",2 )   
        iter+=1
        j+=1
        #plt.draw()
        #plt.show()
    
    """
    sorted_time= sumDiffRates.keys()
    sorted_val= sumDiffRates.values()
    
    #print(  sorted_time,sorted_val )
    
    plt.plot(sorted_time, sorted_val, color=colors[i], label=str(key))
    i = (i+1)%len(colors)
    
    plt.xlabel('Time in seconds')
    plt.ylabel('Rate in Mbps')
    plt.title('%s_rates' %(pre) )
    #plt.legend(loc='upper right')
    plt.savefig('%s/%s.%s.png' %(pre,pre,"rates"))
    plt.draw()
    plt.show()
    
    hosts = list(xrange(144))
    leafs = list(xrange(144,153))
    
    plt.figure(2)
    plt.title('%s_edgelink_prices' %(pre) )
    i=0
    for key in qprices:
        parts = key.split('_');
        print(parts)
        if((int(parts[1]) in hosts) or (int(parts[2]) in hosts)):
          print("edge key %s" %key)
          plt.plot(qtimes[key], qprices[key], colors[i], label=str(key))
          i = (i+1)%len(colors)
    #plt.plot(qx1, qy1, 'k', label="switch1")
    plt.xlabel('Time in seconds')
    plt.ylabel('Queue Price Edge')
    #plt.legend(loc='lower right', prop={'size':6})
    plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_prices_edge"))
    plt.draw()
    
    plt.figure(5)
    plt.title('%s_leaflink_prices' %(pre) )
    i=0
    for key in qprices:
        parts = key.split('_');
        if(not((int(parts[1]) in hosts) or (int(parts[2]) in hosts))):
          print("leaf key %s" %key)
          plt.plot(qtimes[key], qprices[key], colors[i], label=str(key))
          i = (i+1)%len(colors)
    #plt.plot(qx1, qy1, 'k', label="switch1")
    plt.xlabel('Time in seconds')
    plt.ylabel('Queue Price')
    #plt.legend(loc='lower right', prop={'size':6})
    plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_prices"))
    plt.draw()
    
    plt.figure(3)
    plt.title('%s_leaflink_minresidues' %(pre) )
    i=0
    for key in qprices:
        parts = key.split('_');
        if(not((int(parts[1]) in hosts) or (int(parts[2]) in hosts))):
          plt.plot(qtimes[key], qminresidues[key], colors[i], label=str(key))
          i = (i+1)%len(colors)
    #plt.plot(qx1, qy1, 'k', label="switch1")
    plt.xlabel('Time in seconds')
    plt.ylabel('Queue MinResidues')
    #plt.legend(loc='lower right', prop={'size':6})
    plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_minresidues_leaf"))
    plt.draw()
    
    plt.figure(7)
    plt.title('%s_edgelink_minresidues' %(pre) )
    i=0
    for key in qprices:
        parts = key.split('_');
        if((int(parts[1]) in hosts) or (int(parts[2]) in hosts)):
          plt.plot(qtimes[key], qminresidues[key], colors[i], label=str(key))
          i = (i+1)%len(colors)
    #plt.plot(qx1, qy1, 'k', label="switch1")
    plt.xlabel('Time in seconds')
    plt.ylabel('Queue MinResidues')
    #plt.legend(loc='lower right', prop={'size':6})
    plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_residues_edge"))
    plt.draw()
    
    plt.figure(4)
    plt.title('%s_edgelink_utils' %(pre) )
    i=0
    for key in qprices:
        parts = key.split("_")
        if((int(parts[1]) in hosts) or (int(parts[2]) in hosts)):
          plt.plot(qtimes[key], qutils[key], colors[i], label=str(key))
          i = (i+1)%len(colors)
    #plt.plot(qx1, qy1, 'k', label="switch1")
    plt.ylim(0,1.2)
    plt.xlabel('Time in seconds')
    plt.ylabel('Queue Utils')
    #plt.legend(loc='lower right', prop={'size':6})
    plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_utils_edge"))
    plt.draw()
    
    plt.figure(6)
    plt.title('%s_leaflink_utils' %(pre) )
    i=0
    for key in qprices:
        parts = key.split("_")
        if((int(parts[1]) in hosts) or (int(parts[2]) in hosts)):
          plt.plot(qtimes[key], qutils[key], colors[i], label=str(key))
          i = (i+1)%len(colors)
    #plt.plot(qx1, qy1, 'k', label="switch1")
    plt.ylim(0,1.2)
    plt.xlabel('Time in seconds')
    plt.ylabel('Queue Rates')
    #plt.legend(loc='lower right', prop={'size':6})
    plt.savefig('%s/%s.%s.png' %(pre,pre,"queue_utils_leaf"))
    plt.draw()
    
    #dinesh
    #eta=10.0
    #second_term = {}
    #for key in qprices:
    #    second_term[key]=SecondTerm(qprices[key],qutils[key],eta)
    
    
    #mp.rcParams.update({"font.size":8})
    
    #j=1;
    #for key in qtimes: 
    #  i=0
    #  plt.figure(j)
    #  plt.subplot(2,1,1)
    #  plt.plot(qtimes[key], ewma(qprices[key], 1.0), color=colors[i], label=str(" actual price "))
    #  i = (i+1)%len(colors)
    #  new_vec=[0.0];
    #  new_vec.extend(ewma(qprices[key],1.0))
    #  del new_vec[-1] 
    #  plt.plot(qtimes[key], new_vec, color=colors[i], label=str(" prev price "))
    #  i = (i+1)%len(colors)
    #  plt.plot(qtimes[key], ewma(second_term[key], 1.0), color=colors[i], label=str(" util contribution" ))
    #  i = (i+1)%len(colors)
    #  plt.plot(qtimes[key], ewma(qminresidues[key], 1.0), color=colors[i], label=str(" min residue contri  "))
    #  i = (i+1)%len(colors)
    #  j+=1
    #  plt.xlabel('Time in seconds')
    #  plt.ylabel('Rate in Mbps')
    #  plt.title('%s_%s' %(pre, key) )
    #  plt.legend(loc='best')
    
    #  plt.subplot(2,1,2)
    #  plt.plot(qtimes[key], qsizes[key], color=colors[i], label=str(" queue size "))
    #  j+=1
    #  plt.xlabel('Time in seconds')
    #  plt.ylabel('Queue size in bytes')
    #  plt.title('%s_%s' %(pre, key) )
    #  plt.legend(loc='best')
    #  plt.savefig('%s/%s.%s_%d.png' %(pre,pre,"rates",j))
    #  plt.close()
    #
    #"""
    #i=0
    #j=1
    #for key in qtimes: 
    #  plt.figure(j)
    #  plt.plot(qtimes[key], qsizes[key], color=colors[i], label=str(" queue size "))
    #  j+=1
    #  plt.xlabel('Time in seconds')
    #  plt.ylabel('Queue size in bytes')
    #  plt.title('%s_%s' %(pre, key) )
    #  plt.legend(loc='best')
    #  plt.savefig('%s/%s.%s_%d.png' %(pre,pre,"qsizes",j))
    #  plt.close()
    #
    #"""
    ## end DINESH
    #plt.show()
    #
    f.close()
   
if __name__=='__main__':
    main(sys.argv[1], sys.argv[2],sys.argv[3],sys.argv[4])
    plt.draw()
    plt.show()
