import sys
import cvxopt as cvx
import random
import matplotlib;
matplotlib.use('Agg');
del matplotlib;
import matplotlib.pyplot as plt
import numpy as np

###################### Global constants ########################
num_instances = 10
#num_iterations = 10000
num_iterations = 100
max_capacity = 1.0
delay = 0
gamma = 0.1
smoothing = 0.0
tol = 1e-6
epsilon = sys.float_info.epsilon
################################################################

class UtilMax:
    def __init__(self, routes, w, c, method='gradient'):
        
        ########################## Inputs ##############################        
        (self.num_flows, self.num_links) = routes.shape
        self.routes = routes
        self.w = w
        self.c = c
        self.method = method
        ################################################################
        # time
        self.t = 0                                            
        # sending rates
        self.x = np.zeros((self.num_flows,1))        
        # link prices
        self.pr = np.zeros((self.num_links,1))
        # ratios
        #self.ratios = self.Udiff_inv(self.routes.dot(self.pr))
        self.ratios = self.Udiff_inv(np.dot(self.routes,self.pr))
        # timeseries
        self.xs = self.x
        self.prs = self.pr

        print("routes :")
        print(self.routes)
        print(" w :")
        print(w)
        print(" c : ")
        print (c)
        
        
    def U(self, x):
        return self.w*np.log(x)

    def Udiff(self, x):
        with np.errstate(divide='ignore'):        
            val = np.where(x>0.0, self.w/x, np.inf)
        return val

    def Udiff_inv(self, q):
        with np.errstate(divide='ignore'):        
            val = np.where(q>0.0, self.w/q, np.inf)
        return np.where(val<max_capacity, val, max_capacity)
    
    def delay(self, xs):
        return xs[max(-delay-1, -len(xs))]
    
    def compute_link_price(self, g, x, cap, use_cap=False):
        if use_cap == False:
            return min(g)
        
        gx = zip(g,x)
        sorted_gx = sorted(gx, reverse=True)   
        pr = 0
        y = 0
        for (gi,xi) in sorted_gx:
            y += xi
            if y > cap:
                pr = gi
                break
        return pr
    
    def update_prices(self, x, kind='basic', gamma=0.0, use_cap=False):
        new_pr = np.zeros((self.num_links,1))
        marginals = self.Udiff(self.x)
#        y = self.routes.T.dot(self.x)
        y = np.dot(self.routes.T,self.x)

        for l in range(self.num_links):
            index = np.nonzero(self.routes[:,l])[0]
            if not index.tolist(): # skip link if there are no flow crossing it
                new_pr[l] = 0.0
                continue
                
            if y[l] < self.c[l] - 10*epsilon:
                new_pr[l] = 0.0
            else:
                new_pr[l] = self.compute_link_price(marginals[index], 
                                                    self.x[index],
                                                    self.c[l],
                                                    use_cap)
            #if y[l] < self.c[l]:
            #    fictitious_marginal = 1e-6 / (self.c[l]-y[l])
            #    new_pr[l] = min(new_pr[l], fictitious_marginal)
        
            if kind == 'subtract':
                marginals[index] = marginals[index] - new_pr[l]
                                            
        self.pr = gamma*self.pr + (1-gamma)*new_pr

    def update_prices2(self, x, gamma=0.0):
        new_pr = np.zeros((self.num_links,1))
        #error = self.Udiff(self.x) - self.routes.dot(self.pr)
        error = self.Udiff(self.x) - np.dot(self.routes, self.pr)
        marginals = self.Udiff(self.x)
        #y = self.routes.T.dot(self.x)
        y = np.dot(self.routes.T, self.x)

        #if self.t >= 505 and self.t <= 510:
        #    print '*************************'
        #    print 't=', self.t
        #    print 'self.x=', self.x
        #    print 'self.pr=', self.pr        
        
        for l in range(self.num_links):
            index = np.nonzero(self.routes[:,l])[0]
            if not index.tolist(): # skip link if there are no flow crossing it
                new_pr[l] = 0.0
                continue   
            #if self.t >= 505 and self.t <= 510:
            #    print 'link l=', l
            #    print 'y[l]=', y[l]
            
            estimates = np.where(error[index] + self.pr[l] > 0, error[index] + self.pr[l], 0.0)            
            new_pr[l] = np.min(estimates) + 10*(y[l]-self.c[l])
        
            #if y[l] < self.c[l]-10*epsilon:
            #        new_pr[l] = 0.0
            #else:

                #if np.mod(self.t,2) == 1:
                #    new_pr[l] = min(estimates)
                #else:
                #    new_pr[l] = max(estimates)

            #if y[l] < self.c[l]:
            #    fictitious_marginal = 0; #1e-30 / (self.c[l]-y[l])
            #    estimates = np.row_stack((estimates, fictitious_marginal))      
                                     
                
            #if self.t >= 505 and self.t <= 510:
            #    print 'estimates=', estimates
                

       
            new_pr[l] = max(new_pr[l], 0.0)
            #if self.t >= 505 and self.t <= 510:
            #    print 'new_pr[l]=', new_pr[l]
        
        self.pr = gamma*self.pr + (1-gamma)*new_pr
                     
    def update_rates(self, q, gamma=0.0): 
        self.ratios = gamma*self.ratios + (1-gamma)*self.Udiff_inv(q)
        weights = np.array(self.ratios, copy=True)
        self.x = np.zeros((self.num_flows,1))
        rem_flows = np.array(range(self.num_flows))
        rem_cap = np.array(self.c, copy=True)   
        while rem_flows.size != 0:
            #link_weights = self.routes.T.dot(weights)
            link_weights = np.dot(self.routes.T, weights)
            with np.errstate(divide='ignore', invalid='ignore'):
                bl = np.argmax(np.where(link_weights>0.0, link_weights/rem_cap, -1))
            inc = rem_cap[bl]/link_weights[bl]
            self.x[rem_flows] = self.x[rem_flows] + inc*weights[rem_flows]                
            rem_cap = rem_cap - inc*link_weights
            rem_cap = np.where(rem_cap>0.0, rem_cap, 0.0)       
            bf = np.nonzero(self.routes[:,bl])[0]
            rem_flows = np.array([f for f in rem_flows if f not in bf])
            print "rem_flows "
            print rem_flows
            print "bf = "
            print bf
            weights[bf] = 0
                                                                                    
    def set_weights(self, w):
        self.w = w
            
    def step(self):
        self.t += 1
        
        if self.method == 'mp-min':
            #self.update_rates(self.routes.dot(self.pr), gamma=smoothing)
            self.update_rates(np.dot(self.routes, self.pr), gamma=smoothing)
            self.update_prices(self.x, kind='basic', gamma=smoothing, use_cap=False)        
        elif self.method == 'mp-minmax':
            #self.update_rates(self.routes.dot(self.pr), gamma=0.0)
            self.update_rates(np.dot(self.routes, self.pr), gamma=0.0)
            self.update_prices2(self.x, gamma=0.5)    
        else:
            #self.x = self.Udiff_inv(self.routes.dot(self.pr))
            self.x = self.Udiff_inv(np.dot(self.routes, self.pr))
            #self.pr = self.pr + gamma*(self.routes.T.dot(self.x) - self.c)
            print("routes.T")
            print(self.routes.T)
            print("x---")
            print(self.x)
            print("c----")
            print(self.c)
            
            dotproduct = np.dot(self.routes.T, self.x)
            print(dotproduct)
            self.pr = self.pr + gamma*(np.dot(self.routes.T, self.x) - self.c)
            self.pr = np.where(self.pr > 0, self.pr, 0)
            
        self.xs = np.column_stack((self.xs, self.x))
        self.prs = np.column_stack((self.prs, self.pr))
        

def ewma(values, g=1.0/8):
    (m,n) = values.shape
    ret = np.zeros((m,n))
    ret[:,0] = values[:,0]
    for j in range(1,n):
        ret[:,j] = (1.0-g)*ret[:,j-1] + g*values[:,j]
    return ret

def gen_bipartite_instance(nports, nflows):
    #A = np.zeros((nflows+2*nports, 2*nports))
    A = np.zeros((nflows, 2*nports))
    for i in range(nflows):
        src = np.random.randint(nports)
        dst = np.random.randint(nports)
        A[i, src] = 1
        A[i, nports+dst] = 1
    #for i in range(2*nports):
    #    A[nflows+i, i] = 1
    #w = np.row_stack((np.random.rand(nflows, 1), 1e-3*np.ones((2*nports,1))))
    w = np.random.rand(nflows, 1)
    c = np.ones((2*nports, 1))
    return A,w,c

#def main_solver(numports, numflows):
def main_solver():
    #nports = numports;
    #nflows = numflows
    np.random.seed(9852352)  #241
    #plt.close("all")
    #A = np.array([[1,1],
    #             [1,0],
    #             [0,1]])
 #   w = np.array([[5.0, 7.0, 12.0, 11.0]]).T
    # KN
    # w = np.array([[5.0, 7.0, 8.0, 12.0, 11.0]]).T
    #w = np.array([[1.0, 1.0, 1.0, 1.0, 1.0]]).T
    
    # KN 
    # c = np.array([[1.0, 1.0, 1.0, 1.0, 1.0, 1.0]]).T

    # KN
    #A = np.array ([[1,0,1,0,0,0],
    #              [1,1,0,1,0,0],
    #              [0,0,0,0,1,1],
    #              [1,1,0,0,1,0],
    #              [0,1,0,0,1,0]])
    #
    #A = np.array([[1,1,1,1,1],
    #              [1,0,0,0,0],
    #              [0,1,1,0,0],
    #              [0,0,1,1,0],
    #              [0,0,0,0,1],
    #              [0,1,0,1,0],
    #              [1,0,1,0,1],
    #              [0,0,1,0,0]])       
    #w = np.array([[1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]]).T
    #c = np.array([[1.0, 1.0, 1.0, 1.0, 1.0]]).T

#    for inst in range(num_instances):
#    (A, w, c) = gen_bipartite_instance(8, 13)
    
#    if inst !=  5:#!= 15:# and inst != 8:
#        continue

    # at starbux
    ##################################################################
    # SC: 10 flows
    #w = np.array([[1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]]).T
    
    # SC: 13 links 
    #c = np.array([[1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]]).T

    #num_flows = 10
    #num_links = 13

    #w = np.ones((num_flows,1))
    #c = np.ones((num_links,1))    
    
    #A = np.zeros((num_flows,num_links))
   
    #A[0,0] = 1
    #A[1,0] = 1
    #A[2,0] = 1
    #A[2,1] = 1
    #A[3,5] = 1
    #A[3,6] = 1
    #A[4,4] = 1
    #A[5,4] = 1
    #A[6,8] = 1
    #A[7,9] = 1
    #A[8,11] = 1
    #A[8,12] = 1
    #A[9,12] = 1
    #print(A)

    # more complex with bottlenecks
    ###################################################################
    # SC: 11 flows
    w = np.array([[1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]]).T
    #
    ## SC: 13 links 
    c = np.array([[1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]]).T

    num_flows = 11
    num_links = 13
    A = np.zeros((num_flows,num_links))
   
    A[0,0] = 1
    ##
    A[1,0] = 1
    A[1,1] = 1
    A[1,6] = 1
    A[1,7] = 1
    A[1,9] = 1
    A[1,10] = 1
    A[1,11] = 1
    A[1,12] = 1
    #
    A[2,0] = 1
    A[2,1] = 1
    A[2,5] = 1
    ##
    #A[3,4] = 1
    #A[3,2] = 1
    A[3,3] = 1
    ## 
    A[4,4] = 1
    ##
    A[5,6] = 1
    A[5,7] = 1
    ## 
    A[6,7] = 1
    ## 
    A[7,9] = 1
    A[7,10] = 1
    A[7,11] = 1
    A[7,12] = 1
    ##
    A[8,7] = 1
    A[8,8] = 1
    ##
    A[9,10] = 1
    A[9,11] = 1
    A[9,12] = 1
    ##
    A[10,11] = 1
   
    print(A)

    
    umax_gradient = UtilMax(A, w, c, method='gradient')
    umax_mp = UtilMax(A, w, c, method='mp-minmax')


    for it in range(num_iterations):
       umax_gradient.step()
       umax_mp.step()
        
    #rate_error = np.linalg.norm(umax_gradient.x - umax_gradient.x)/umax_gradient.num_flows
    #rate_error = np.linalg.norm(umax_gradient.x - umax_gradient.x)/umax_gradient.num_flows
    if 1:#rate_error > tol:
#        print '\n\n###################### instance %d #######################' % inst
        print 'nflows=', A.shape[0]
        print 'nports=', A.shape[1]/2
        print 'A='
        print A
        print 'w='
        print w.T
        print 'c='
        print c.T
        print '**********************************************************'
        print 'gradient x='
        print umax_gradient.x.T        
        print 'gradient pr='
        print umax_gradient.pr.T
#        print 'gradient q='
        #print umax_gradient.routes.dot(umax_gradient.pr).T
#        print np.dot(umax_gradient.routes,umax_gradient.pr).T
#        print 'gradient marginals=', 
#        print  umax_gradient.Udiff(umax_gradient.x).T   
#        print 'gradient kkt error=', np.linalg.norm(np.dot(umax_gradient.routes,umax_gradient.pr) - umax_gradient.Udiff(umax_gradient.x))
        print '--'
        print 'mp x='
        print umax_mp.x.T
        print 'mp pr='
        print umax_mp.pr.T
        #print 'mp q='
        #print umax_mp.routes.dot(umax_mp.pr).T
        #print np.dot(umax_mp.routes, umax_mp.pr).T
        #print 'mp marginals='
        #print  umax_mp.Udiff(umax_mp.x).T
        #print 'mp kkt error=', np.linalg.norm(umax_mp.routes.dot(umax_mp.pr) - umax_mp.Udiff(umax_mp.x))
        #print 'mp kkt error=', np.linalg.norm(np.dot(umax_mp.routes, umax_mp.pr) - umax_mp.Udiff(umax_mp.x))
        #print '-----> rate error=', rate_error
         
        #smoothed_xs = ewma(umax_mp.xs, g=0.01)
        #smoothed_prs = ewma(umax_mp.prs, g=0.01)
        #final_smooth_pr = np.reshape(smoothed_prs[:,-1], (-1,1))
        #final_smooth_x = np.reshape(smoothed_xs[:,-1], (-1,1))
        #print '---------------------------------------'
        #print 'final smoothed_x=', smoothed_xs[:,-1]
        #print 'final smoothed_pr=', smoothed_prs[:,-1]
        #print 'marginals=', umax_mp.Udiff(final_smooth_x).T
        #print 'final smoothed_error=', np.linalg.norm(umax_mp.routes.dot(final_smooth_pr) - umax_mp.Udiff(final_smooth_x))
        #                    
        
        #plt.figure('instance  -- link rate')
        #plt.figure(3)
        #plt.title('link rates')
        #plt.plot(np.dot(umax_mp.xs.T, umax_mp.routes))
        #plt.plot(np.dot(umax_gradient.xs.T, umax_mp.routes), '--')
        ##plt.plot(umax_mp.xs.T.dot(umax_mp.routes))
        #plt.plot(umax_gradient.xs.T.dot(umax_mp.routes), '--')
        #plt.legend(range(umax_mp.num_links))
        #plt.show()
                
        #plt.figure('instance %d -- price' % inst)
        #plt.figure(4)
        #plt.title('link prices')
        #plt.plot(umax_mp.prs.T)
        #plt.plot(umax_gradient.prs.T, '--')
        #plt.legend(range(umax_mp.num_links))
        #plt.show()  

        #plt.figure('instance %d -- flow rate' % inst)
        #plt.figure(5)
        #plt.title('flow rates')
        #plt.plot(umax_mp.xs.T)
        #plt.plot(umax_gradient.xs.T, '--')
        #plt.ylim(0,1)
        #plt.legend(range(umax_mp.num_flows))
        #plt.show()
            
#plt.figure(4)
#plt.title('rate error (log scale)')
#plt.plot(np.log(np.abs(state_mp.xs.T - state_gradient.xs.T)))
#plt.legend(range(state_mp.num_flows))
#plt.show()  
#



#plt.figure(5)
#plt.title('smoothed prices')
#plt.plot(smoothed_prs.T)
#plt.legend(range(state.num_links))
#plt.show()
#                              
    
if __name__ == '__main__':
#    main()
    main_solver()


