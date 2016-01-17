import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def main(inputfile,inputdir, fignum, colors):
    # do whatever and return 0 for success and an 
    # integer x, 1 <= x <= 256 for failure

    # Choose how many bins you want here
    num_bins = 20
    
    filename = inputfile+"_cdf"
    pre = inputfile;
    
    if not os.path.exists(inputdir):
      os.makedirs(inputdir)
    
    data = np.loadtxt(filename)
    
    # Use the histogram function to bin the data
    #counts, bin_edges = np.histogram(data, bins=num_bins, normed=True)

    #counts= counts/np.sum(counts)

    # Now find the cdf
    #cdf = np.cumsum(counts)
    
    
    plt.figure(fignum)
    plt.title(pre)
    # And finally plot the cdf
    #plt.plot(bin_edges[1:], cdf, 'k', label=(str(len(data)) +" " + pre))
    
    sorted_data = np.sort(data)
    
    yvals= (1 + np.arange(len(sorted_data)))/float(len(sorted_data))
    print(yvals) 
    plt.plot(sorted_data,yvals, colors, label=(str(len(data)) +" " + pre))
    plt.xlabel('Convergence Time in seconds')
    plt.ylabel('CDF ')
    #plt.legend(loc='best')
    lgd=plt.legend(loc='upper center', bbox_to_anchor=(0.5,-0.1))
    plt.savefig('%s/%s.%s.jpg' %(inputdir,inputfile,"cdf"), bbox_extra_artists=(lgd,), bbox_inches='tight')
    #plt.savefig('%s/%s.%s.jpg' %(inputdir,inputfile,"cdf"))
    #plt.draw()
    #plt.show()
    #plt.close()
if __name__=='__main__':
    sys.exit(main(sys.argv[1], sys.argv[2],sys.argv[3], sys.argv[4]))

#plt.draw()
#plt.show()

#
#gaussian_numbers = np.random.randn(1000)
#plt.hist(gaussian_numbers)
#plt.title("Gaussian Histogram")
#plt.xlabel("Value")
#plt.ylabel("Frequency")
#plt.draw()
#plt.show()
