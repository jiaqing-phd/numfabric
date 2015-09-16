import sys
#import cvxopt as cvx
import random
import matplotlib.pyplot as plt
import numpy as np

###################### Global constants ########################
num_instances = 1
max_iterations = 10000
max_capacity = 1.0
gamma = 0.01
smoothing = 0.0
tol = 1e-10
epsilon = sys.float_info.epsilon
ONEMILLION=1000000
capacity = 1000000000
UMAX_INT = 9999999999

paths = {}
paths[(6,10)]=  [1,0,1,0,0,0]
paths[(7,12)]=  [1,1,0,1,0,0]
paths[(14,15)]= [0,0,0,0,1,1]
paths[(8,16)]=  [1,1,0,0,1,0]
paths[(9,17)]=  [0,1,0,0,1,0]


################################################################
  

class Flow:
  def __init__(self, starttime, flowid, flowsize, srcid, dstid, weight):
    self.starttime = starttime
    self.flowid = flowid
    self.flowsize = flowsize
    self.added = False
    self.srcid = srcid
    self.dstid = dstid
    self.weight = weight

  def printFlow(self):
    print("flow_id %d flow_starttime %f %d %d" %(self.flowid, self.starttime, self.srcid, self.dstid))
 
class event:
  def __init__(self, flow, etype):
    self.flow = flow
    self.event_type = etype
    self.added = False

class UtilMax:
    #def __init__(self, routes, w, c, alpha=1, method='gradient'):
    def __init__(self, numports, method="mp",alpha=1.0):
        
        ########################## Inputs ##############################        
        #(self.num_flows, self.num_links) = routes.shape
        #self.routes = routes
        #print("UtilMax method is %s alpha is %f" %(method, alpha))
        self.alpha = alpha
        self.method = method
        self.num_links = numports
        self.num_flows = 0
        self.routes = np.zeros((self.num_flows, self.num_links))
        self.w = np.ones((self.num_flows, 1))
        self.c = np.ones((self.num_links, 1))
        self.maxdata = np.zeros(self.num_flows)
        ################################################################
        # time
        self.t = ONEMILLION
        ## sending rates
        self.x = np.zeros((self.num_flows,1))        
        # link prices
        self.pr = np.zeros((self.num_links, 1))#np.random.rand(self.num_links, 1) #np.zeros((self.num_links,1))
        # ratios
        self.ratios = self.Udiff_inv(np.dot(self.routes,self.pr))
        #print("Initial ratios.. ")
        #print self.ratios

        # optimality gap
        self.opt_gap = np.inf
        # data sent
        self.data_sent = np.zeros((self.num_flows,1))
        
        
        # timeseries
        #self.xs = self.x
        self.prs = self.pr
        self.opt_gaps = [self.opt_gap]
        self.first_time = True

    def get_snapshot(self):
      return (self.routes, self.ratios, self.data_sent, self.t, self.maxdata, self.w, self.num_flows)

    def util_reinit(self, new_matrix, new_ratios, new_datasent, new_maxdata, weights, num_flows, time):
      self.routes = np.array(new_matrix, copy = True)
      self.ratios = np.array(new_ratios, copy = True)
      self.data_sent = np.array(new_datasent, copy = True)
      self.maxdata = np.array(new_maxdata, copy = True)
      self.w = np.array(weights, copy = True)
      self.num_flows = num_flows
      self.first_time = True
      self.converged = False
      self.x = np.zeros((self.num_flows, 1))

#      print("MaxUtil reinit with routes at time %d.. "%time)
#      print(self.routes)
#      print("MaxUtil reinit with ratios.. ")
#      print(self.ratios)
#      print("MaxUtil reinit with data_sent.. ")
#      print(self.data_sent)
#      print("MaxUtil reinit with maxdata.. ")
#      print(self.maxdata)
#      print("numflows %d"%self.num_flows)

    def rem_flow(self, row_id):
        self.num_flows -= 1
        np.delete(self.routes, row_id, axis=0)  
        # what else to reinitialize? 
        
    def U(self, x):
        if self.alpha == 1:
            val = self.w*np.log(x)
        else:
            val = 1.0/(1-self.alpha)*(self.w**self.alpha)*(x**(1-self.alpha))
        return val

    def Udiff(self, x):
        with np.errstate(divide='ignore'):        
            val = np.where(x>0.0, (self.w/x)**self.alpha, np.inf)
        return val

    def Udiff_inv(self, q):
        with np.errstate(divide='ignore'):        
            val = np.where(q>0.0, self.w*(q**(-1.0/self.alpha)), np.inf)
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
    
    def update_prices(self, x, kind='basic', beta=0.0, use_cap=False):
        new_pr = np.zeros((self.num_links,1))
        marginals = self.Udiff(self.x)
        #y = self.routes.T.dot(self.x)
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
                                            
        self.pr = beta*self.pr + (1-beta)*new_pr

    def update_prices2(self, x, beta=0.0):
        new_pr = np.zeros((self.num_links,1))
        #error = self.Udiff(self.x) - self.routes.dot(self.pr)
        error = self.Udiff(self.x) - np.dot(self.routes, self.pr)
        marginals = self.Udiff(self.x)
        #y = self.routes.T.dot(self.x)
        y = np.dot(self.routes.T, self.x)
               
        for l in range(self.num_links):
            index = np.nonzero(self.routes[:,l])[0]
            if not index.tolist(): # skip link if there are no flow crossing it
                new_pr[l] = 0.0
                continue   
            
            #estimates = np.where(error[index] + self.pr[l] > 0, error[index] + self.pr[l], 0.0)            
            #new_pr[l] = np.min(estimates) + (1.0)*(y[l]-self.c[l])            
            #new_pr[l] = np.min(estimates)*(y[l]/self.c[l])**4 + (1.0)*(y[l]-self.c[l]) 
            new_pr[l] = self.pr[l] + np.min(error[index]) - 10*(1-y[l]/self.c[l])*self.pr[l] #*np.log(1e20/self.pr[l])  #100.0*(y[l]-self.c[l])
            new_pr[l] = max(new_pr[l], 0.0)

        self.pr = beta*self.pr + (1-beta)*new_pr


    def update_rates_maxmin(self): 
        #self.ratios = beta*self.ratios + (1-beta)*self.Udiff_inv(q)
        weights = np.ones((self.num_flows, 1))
        self.x = np.zeros((self.num_flows,1))
        rem_flows = np.array(range(self.num_flows))
        rem_cap = np.array(self.c, copy=True)   
        while rem_flows.size != 0:
            link_weights = np.dot(self.routes.T, weights)
            with np.errstate(divide='ignore', invalid='ignore'):
                bl = np.argmax(np.where(link_weights>0.0, link_weights/rem_cap, -1))
            inc = rem_cap[bl]/link_weights[bl]
            self.x[rem_flows] = self.x[rem_flows] + inc*weights[rem_flows]                
            rem_cap = rem_cap - inc*link_weights
            rem_cap = np.where(rem_cap>0.0, rem_cap, 0.0)       
            bf = np.nonzero(self.routes[:,bl])[0]
            rem_flows = np.array([f for f in rem_flows if f not in bf])
            weights[bf] = 0
        #print("Rates : ..")
        #print(self.x)

                     
    def update_rates(self, q, beta=0.0): 
        self.ratios = beta*self.ratios + (1-beta)*self.Udiff_inv(q)
        weights = np.array(self.ratios, copy=True)
        self.x = np.zeros((self.num_flows,1))
        rem_flows = np.array(range(self.num_flows))
        rem_cap = np.array(self.c, copy=True)   
        while rem_flows.size != 0:
            link_weights = np.dot(self.routes.T, weights)
            with np.errstate(divide='ignore', invalid='ignore'):
                bl = np.argmax(np.where(link_weights>0.0, link_weights/rem_cap, -1))
            inc = rem_cap[bl]/link_weights[bl]
            self.x[rem_flows] = self.x[rem_flows] + inc*weights[rem_flows]                
            rem_cap = rem_cap - inc*link_weights
            rem_cap = np.where(rem_cap>0.0, rem_cap, 0.0)       
            bf = np.nonzero(self.routes[:,bl])[0]
            rem_flows = np.array([f for f in rem_flows if f not in bf])
            weights[bf] = 0

    def updateRates(self): 
        self.converged = False
        converg_it = 0
        max_iterations = 1000
        good = 0
        conseq_good = 50 
        #print(self.routes)
        #print(self.pr)
        if(self.method == "mp"):
            while(self.converged == False and converg_it < max_iterations):
              self.update_rates(np.dot(self.routes, self.pr), beta=0.0)
              self.update_prices2(self.x, beta=0.5)
              primal_opt = np.sum(self.U(self.x))
              dualopt_x = self.Udiff_inv(np.dot(self.routes,self.pr))
              dual_opt = np.dot(self.c.T,self.pr) + np.sum(self.U(dualopt_x)-np.dot(self.routes,self.pr)*dualopt_x)
              self.opt_gap = np.abs(dual_opt - primal_opt)
              if self.opt_gap < tol:   
                good += 1
              else:
                good = 0
              if(good == conseq_good):
                self.converged = True    
                #print("MP Rates : self.converged %d" %self.converged)
                print(self.x)
              converg_it+=1
              #print("Trying for the rates to converge.. iteration %d gap %f" %(converg_it, self.opt_gap))
       ## print rates
        return self.x
        if(self.method == "maxmin"):
            self.update_rates_maxmin()

    def set_weights(self, w):
        self.w = w

  
  
    def update_data_sent(self):
      #time-slot = 1 us
      time_slot=0.000001
      idx = 0
      for frate in self.x:
        # units - capacity in Gbps. Slot in secs. So, in Gb; multiply by 1000 to convert to Mb;
        data_in_this_slot = frate*time_slot*1000000000.0/8.0;
#        print("index = %d data in slot = %f rate %f" %(idx,data_in_this_slot,frate))
        self.data_sent[idx] = 1.0*self.data_sent[idx] + data_in_this_slot; 
        idx +=1 
#        if(self.maxdata[idx] <= self.data_sent[idx]):
          # need to remove the flow from the matrix - how ?  
#          print("%d data sent in %d iterations for flow_id %d" %(self.data_sent[real_id], itr, real_id))
#          self.rem_flow(idx)
            
    def step(self):
        self.t += 1

        if(self.num_flows == 0):
          return
        
        if self.method == 'mp-old':
            self.update_rates(np.dot(self.routes, self.pr), beta=smoothing)
            self.update_prices(self.x, kind='basic', beta=smoothing, use_cap=False)
        elif self.method == 'mp':
            converg_it = 0
            good = 0
            conseq_good = 100
            
            while(self.converged == False and converg_it < max_iterations):
              self.update_rates(np.dot(self.routes, self.pr), beta=0.0)
              self.update_prices2(self.x, beta=0.5)
              primal_opt = np.sum(self.U(self.x))
              dualopt_x = self.Udiff_inv(np.dot(self.routes,self.pr))
              dual_opt = np.dot(self.c.T,self.pr) + np.sum(self.U(dualopt_x)-np.dot(self.routes,self.pr)*dualopt_x)
              self.opt_gap = np.abs(dual_opt - primal_opt)
              if self.opt_gap < tol:   
                good += 1
              else:
                good = 0
              if(good == conseq_good):
                self.converged = True    
                #print("MP Rates : self.converged %d" %self.converged)
                #print(self.x)
              converg_it+=1
              #print("Trying for the rates to converge.. iteration %d gap %f" %(converg_it, self.opt_gap))
              
            self.update_data_sent()
        elif self.method == 'maxmin':
            if(self.first_time):
              self.update_rates_maxmin(np.dot(self.routes, self.pr), beta=0.0)
              self.update_prices2(self.x, beta=0.5)
              self.update_data_sent()
              self.first_time = False;
            else:
              self.update_data_sent()

        else:
            #self.x = self.Udiff_inv(self.routes.dot(self.pr))
            self.x = self.Udiff_inv(np.dot(self.routes, self.pr))
            #self.pr = self.pr + gamma*(self.routes.T.dot(self.x) - self.c)
            self.pr = self.pr + gamma*(np.dot(self.routes.T, self.x) - self.c)
            self.pr = np.where(self.pr > 0, self.pr, 0)

        primal_opt = np.sum(self.U(self.x))
        #dualopt_x = self.Udiff_inv(self.routes.dot(self.pr))
        dualopt_x = self.Udiff_inv(np.dot(self.routes,self.pr))
        #dual_opt = self.c.T.dot(self.pr) + np.sum(self.U(dualopt_x)-self.routes.dot(self.pr)*dualopt_x)
        dual_opt = np.dot(self.c.T,self.pr) + np.sum(self.U(dualopt_x)-np.dot(self.routes,self.pr)*dualopt_x)
        self.opt_gap = np.abs(dual_opt - primal_opt)
                        
        #self.xs = np.column_stack((self.xs, self.x))
        #self.prs = np.column_stack((self.prs, self.pr))
        #self.opt_gaps.append(self.opt_gap)
       
    def solve(self, tol=1e-10, conseq_good=100, max_it=10000):
        it = 0
        good = 0
        while it < max_it and good < conseq_good:
            it += 1
            self.step()

            if self.umax_mp.opt_gap < tol:   
                good += 1
            else:
                good = 0
        return ((good==conseq_good), self.umax_mp.opt_gap, it)

    def print_details(self):
        print 'method=', self.method
        print 'x='
        print self.x.T
        print 'pr='
        print self.pr.T
        print 'q='
        #print self.routes.dot(self.pr).T
        print np.dot(self.routes,self.pr).T
        print 'marginals='
        print  self.Udiff(self.x).T
        print 'kkt error=', np.linalg.norm(np.dot(self.routes, self.pr) - self.Udiff(self.x))
        print 'complementary slackness=', np.linalg.norm((np.dot(self.routes.T, self.x)-self.c)*self.pr)  
        primal_opt = np.sum(self.U(self.x))
        dualopt_x = self.Udiff_inv(np.dot(self.routes, self.pr))
        dual_opt = np.dot(self.c.T, self.pr) + np.sum(self.U(dualopt_x)- np.dot(self.routes, self.pr)*dualopt_x)
        print 'primal optimal=', primal_opt
        print 'dual optimal=', dual_opt
        print 'optimality gap=', np.abs(primal_opt - dual_opt)   

def ewma(values, g=1.0/8):
    (m,n) = values.shape
    ret = np.zeros((m,n))
    ret[:,0] = values[:,0]
    for j in range(1,n):
        ret[:,j] = (1.0-g)*ret[:,j-1] + g*values[:,j]
    return ret

def gen_bipartite_instance(nports, nflows):
    A = np.zeros((nflows, 2*nports))
    for i in range(nflows):
        src = np.random.randint(nports)
        dst = np.random.randint(nports)
        A[i, src] = 1
        A[i, nports+dst] = 1
    w = np.random.rand(nflows, 1)
    c = np.ones((2*nports, 1))
    return A,w,c

def gen_A_from_B(B):
  temp = []
  n = 0
  for r in B.tolist():
    if 1 in r:
      temp.append(r)
      n += 1 
  ret = np.asarray(temp)
  w = np.ones((n, 1))
  return (ret, n, w)

   
 
class Simulation:

  def init_custom(self, nports, meth, alpha_val):
    self.Flows = []
    self.numports = nports
    self.events = []
    if(meth == "mp"):
        self.umax_mp = UtilMax(self.numports, method=meth, alpha=alpha_val)
    elif(meth ==  "alpha_dif"):
        self.umax_mp = UtilMax(self.numports, method=meth, alpha=0.25)
    else:
        self.umax_mp = UtilMax(self.numports, method=meth, alpha=1.0)
        
        
    self.realids = {}
    self.method = meth
    

  def add_row(self, new_row, flow_size, fid, new_weight):
    # get a snapshot of the existing prices, ratios and data_sent and re-init the object
    (routes, ratios, data_sent, time, maxdata,w, num_flows) = self.umax_mp.get_snapshot()
    #print("adding row ")
    #print new_row

    if(num_flows == 0):
      new_matrix = new_row
      new_ratios = [[0.0]]
      new_datasent = [[0.0]]
      new_maxdata = [[flow_size]]
      weight = [[new_weight]]
      self.real_id = np.array([fid])
    else:
      new_matrix =  np.vstack([routes, new_row])
      new_ratios = np.vstack([ratios, [0.0]])
      new_datasent = np.vstack([data_sent, [0.0]])
      new_maxdata = np.vstack([maxdata, [flow_size]])
      weight = np.vstack([w, [new_weight]])
      self.real_id = np.vstack([self.real_id, [fid]])
      #print("realids...")
      #print(self.real_id)
    # mapping from real id to row number in current matrix
    num_flows += 1
  
    self.umax_mp.util_reinit(new_matrix, new_ratios, new_datasent, new_maxdata, weight, num_flows, self.it)
  
  # function called from the parsing script to form a list of flows   
  #def add_flow_list(self, flow_id, flow_size, flow_arrival, srcid, dstid, weight):
    # a list of flows sorted according to their arrivals
    ##print("adding flow with classid %d flow_id %d flow_size %d flow_arrival %f" %(class_id, flow_id, flow_size, flow_arrival*ONEMILLION))
    #f = Flow(flow_arrival/1000.0, flow_id, flow_size, srcid, dstid, weight)
    #self.Flows.append(f)

  def add_event_list(self, flow_id, flow_size, time, srcid, dstid, weight, event_type):
    print("adding event flow at time %f event_type %d" %(time, event_type))
    f = Flow(time/1000.0, flow_id, flow_size, srcid, dstid, weight)
    new_event = event(f, event_type)
    self.events.append(new_event)

  def addFlow(self, f):
    #print("flow being inserted.. flow id %d classid %d "%(f.flowid, f.classid))
    new_row = np.zeros((1, self.numports))
    print("addFlow src %d dst %d" %(f.srcid, f.dstid))
    new_row = np.asmatrix(paths[(f.srcid, f.dstid)])
    #print("adding new_row")
    #print(new_row)
    self.add_row(new_row, f.flowsize, f.flowid, f.weight)
    f.added = True

  def removeFlow(self, f):
   # flow_index = self.reverse_map[f.flowid]
    flow_index = -1
    index = 0
    for fid in self.real_id:
      if(fid == f.flowid):
        flow_index = index
      index+=1

    #print("flow id %d row index %d dropping" %(f.flowid, flow_index))
    self.dropFlow(flow_index)

  def getFlowWithIndex(self, fidx):
    flowid = self.real_id[fidx]
    
    for f in self.Flows:
      if(f.flowid == flowid):
        return f

  def dropFlow(self, fidx):
    # remove from the matrix and remove from the flow list
    #self.umax_mp.routes = np.delete(self.umax_mp.routes, fidx, axis=0)
    # find the flow corresponding to this row in Flows
    #print("dropFlow : fidx %d "%fidx)
    #print("realids.. ")
    #print(self.real_id)
    flowid = self.real_id[fidx]
    self.real_id = np.delete(self.real_id, fidx, axis=0)

    for f in self.Flows:
      if(f.flowid == flowid):
        #print("removing flow with index %d realid %d at time %f: time_taken %f flow_size %d flow_start %d" %(fidx, f.flowid, self.it/ONEMILLION, (self.it-f.starttime)/ONEMILLION, f.flowsize, f.starttime/ONEMILLION))
        self.Flows.remove(f)

    (routes, ratios, data_sent, time, maxdata,w, num_flows) = self.umax_mp.get_snapshot()
    #print("deleting row %d at time %f" %(fidx, time))

    new_matrix =  np.delete(routes, fidx, axis=0)
    new_ratios = np.delete(ratios, fidx, axis=0)
    new_datasent = np.delete(data_sent, fidx, axis=0)
    new_maxdata = np.delete(maxdata, fidx, axis=0)
    weight = np.delete(w, fidx, axis=0)
    num_flows -= 1
  
    self.umax_mp.util_reinit(new_matrix, new_ratios, new_datasent, new_maxdata, weight, num_flows, self.it)

  def flow_has_traffic(self):
    for fidx in range(0, len(self.umax_mp.data_sent)):
      if(self.umax_mp.data_sent[fidx] < self.umax_mp.maxdata[fidx]):
        return True
    return False

  def event_pending(self):
    if(len(self.events) > 0):
      for f in self.events:
        if(f.added == False):
          return True
    return False

  def flow_pending(self):
    if(len(self.Flows) > 0):
      for f in self.Flows:
        if(f.added == False):
          return True
      #print("flow is pending...%d" %len(self.Flows))
      #if(len(self.Flows) == 1):
      #  for f in self.Flows:
      #    f.printFlow()
    #print("no flows pending.. returning False")
    return False

  def get_next_arrival(self):
    for f in self.Flows:
      if(f.starttime >= self.it and f.added == False):
        #print("flow id %d iteration %d arrival %d" %(f.flowid, self.it, f.starttime))
        return (f, f.starttime)

    # there are no more arrivals
    return (f, -1)

  def get_next_event(self):
    #print("get_next_event.. ")
    for e in self.events:
      if(e.flow.starttime >= self.it and e.added == False):
        #print("returning... %f" %e.flow.starttime)
        return (e, e.event_type)

    return (e , -1)
      

  def get_earliest_finish(self, datasent):
    #for fidx in range(0, (self.umax_mp.data_sent.shape)[0]):
    min_finish_time = UMAX_INT
    min_finish_fid = -1
    fidx = 0
    for frate in self.umax_mp.x:
      if(datasent[fidx] >= self.umax_mp.maxdata[fidx]):
      # this flow will finish .. but exactly when?
        data_remaining = self.umax_mp.maxdata[fidx] - self.umax_mp.data_sent[fidx]
        #print("data_remaining %d maxdata %d data_sent %d" %(data_remaining, self.umax_mp.maxdata[fidx],self.umax_mp.data_sent[fidx]))
        #print("rate = %f" %frate)
        time_to_send = data_remaining * 8.0 * ONEMILLION/ (frate * capacity)
        if(min_finish_time > time_to_send):
          min_finish_time = time_to_send
          min_finish_fid = fidx
    #print("the flow that'll finish fastest %d %d "%(min_finish_time, min_finish_fid))
    return (min_finish_time, min_finish_fid)

  def get_shortest_remaining_flow(self):
    #for fidx in range(0, (self.umax_mp.data_sent.shape)[0]):
    min_finish_time = UMAX_INT
    min_finish_fid = -1
    fidx = 0
    for frate in self.umax_mp.x:
      data_remaining = self.umax_mp.maxdata[fidx] - self.umax_mp.data_sent[fidx]
      #print("get_shortest: remaining data %d for flow %d at time %d"%(data_remaining,fidx,self.it))
      time_to_send = self.it + (data_remaining * 8.0 * ONEMILLION / (frate * capacity))
      if(min_finish_time > time_to_send):
        min_finish_time = time_to_send
        min_finish_fid = fidx
      fidx+=1
    #print("get_shortest_remaining_flow: the flow that'll finish soonest %d at time %f"%(min_finish_fid, min_finish_time))
    return (min_finish_time, min_finish_fid)


  def update_all_data_sent(self, time, for_time):
    idx = 0
    #print("update_all_data_sent : fortime %f"%for_time)
    class_rates = {}
    num_flows = {}
    total_rate = 0
    total_numflows = 0
    total_rate_long = 0
    total_rate_short = 0

    #for key in (1,2,3):
    #  class_rates[key] = 0
    #  num_flows[key] = 0 

    for_time /= ONEMILLION * 1.0
    for frate in self.umax_mp.x:
      f = self.getFlowWithIndex(idx)
      #classid = f.classid
      #if(classid not in class_rates):
      #  class_rates[classid] = 0
      #  num_flows[classid] = 0
      #class_rates[classid] += frate 
      #num_flows[classid] += 1
      #if(classid == 0):
      #  total_rate_long += frate
      #else:
      #  total_rate_short += frate
      #total_rate += frate
      #total_numflows += 1

      #print("time %f flow %d datarate %f classid %d" %(time/ONEMILLION, self.real_id[idx],frate, classid))
      self.umax_mp.data_sent[idx] = 1.0*self.umax_mp.data_sent[idx] + (frate * for_time * capacity/8.0);
      idx +=1

    ######## STATS ##########
    for key in class_rates:
      print("class_rates time %f class %d rate %f num_flows %d total_rate %f"%(time/ONEMILLION, key, class_rates[key], num_flows[key], total_rate))


    #if(num_flows[1] != 0 or num_flows[2] !=0 or num_flows[3] != 0): 
       
    #  r1n1=class_rates[1]
    #  r2n2=class_rates[2]
    #  r3n3=class_rates[3]

    #  if((r1n1 - (float(num_flows[1])/float(num_flows[1]+num_flows[2]+num_flows[3]))) > 0.00001):
    #    print("r1n1 sanity doesn't check")
#      if((r1n1 -num_flows[1]*1.0/(1.0*float((1.0*num_flows[1] + float(max(num_flows[2], num_flows[3]))))))> 0.0000001):
#        print("r1n1 sanity doesn't check..look stats above %f %d %d %d %f" %(r1n1, num_flows[1], num_flows[2], num_flows[3], (1.0*float((1.0*num_flows[1] + float(max(num_flows[2], num_flows[3]))))) ))
    #  if((r2n2 - (1-r1n1)) > 0.00000001):
    #    print("r2n2 sanity doesn't check..look stats above %f %f" %(r2n2, (1-r1n1)))
    

    #print("total_rate %f %f %d %f %f" %(time/ONEMILLION, total_rate, total_numflows, total_rate_long, total_rate_short))
      
      

  def execute_next_event(self):
    (next_flow, event_type) = self.get_next_event();
    #print("next flowid %d arrival at %d" %(next_flow.flowid, arrival))
    #(finish_time, finish_fid) = (UMAX_INT, -1)
    #(finish_time, finish_fid) = self.get_shortest_remaining_flow()

    #old_time = self.it 
    #earliest flow to finish will finish at finish_time
    #if(finish_time < arrival or (arrival == -1 and finish_fid != -1)):
      #flow finish of flow id fid is the next event
      #self.it = finish_time
      #time_diff = self.it - old_time
      #self.update_all_data_sent(self.it, time_diff)
      #print("advancing iterations from %d to %d" %(old_time, self.it))
      #print("finish_time %d arrival %d" %(finish_time, arrival))
      #self.dropFlow(finish_fid)
    #elif(arrival != -1):
    #  self.it = arrival
    #  time_diff = self.it - old_time
    #  self.update_all_data_sent(self.it, time_diff)
    #print("event_type %d" %event_type)
    if(event_type == 1):
      self.addFlow(next_flow.flow)
      next_flow.added  = True
    if(event_type == 2):
      self.removeFlow(next_flow.flow)
      next_flow.added = True
      #print("advancing iterations from %d to %d" %(old_time, self.it))
    self.converged = False
    self.first_time = False

  def get_class_nums(self):
    class_nums = {}
    for flow_id in self.real_id:
      f = self.get_flow_with_realid(flow_id)
      class_id = f.classid
      if(class_id not in class_nums):
        class_nums[class_id] = 0
      class_nums[class_id] += 1 
    return class_nums

  def get_flow_with_realid(self, real_id):
    for f in self.Flows:
      if(f.flowid == real_id):
        return f
      

  def update_maxthr_rates(self):
    class_nums = self.get_class_nums()
    class_rates = {}
    class1_only = True
    for key in class_nums:
      #print("update_maxthr_rates : key %d class_nums %d" %(key, class_nums[key]))
      if(key > 0 and class_nums[key] > 0):
        # there are flows in category higher than the longest flow
        class_rates[0] = 0
        class_rates[key] = 1.0
        class1_only = False
        
    if(class1_only == True):
        class_rates[0] = 1.0
    ## classes go thier rates. Now give it to flows
    index = 0
    for flow_id in self.real_id:
      f = self.get_flow_with_realid(flow_id)
      class_id = f.classid
      #print("update_maxthr_rates : flow_id %d class_id %d class_num %d class_rate %f"%(flow_id, class_id, class_nums[class_id], class_rates[class_id]))
      _rate = class_rates[class_id]/class_nums[class_id]
      self.umax_mp.x[index] = _rate
      index +=1

  def update_rates_wrapper(self):
    if(self.method == "mp" or self.method == "maxmin"):
      rates = self.umax_mp.updateRates()
    if(self.method == "maxthroughput"):
      rates = self.update_maxthr_rates()
    return rates

  def sort_flows_on_arrivaltime(self):
    self.Flows = sorted(self.Flows,  key=lambda flow_object: flow_object.starttime)
    
    #print("printing sorted flows")
    #for f in self.Flows:
    #  f.printFlow()

  def startSim(self):
    self.it = 0 #ONEMILLION
    good = 0
    conseq_good = 100
    tol = 1e-10
    max_it = 100
    return_rates = {} 
    #while it < max_it and good < conseq_good:
    #print("Starting simulation... ")
    #self.sort_flows_on_arrivaltime()
    #while(self.flow_has_traffic() or self.flow_pending()):
    while(self.event_pending()):
       #self.it += 1
       #self.umax_mp.step()
       self.execute_next_event()
       rates = self.update_rates_wrapper()
       
       idx = 0
       for rate in rates:
         #print("flow %d rate %f" %(self.real_id[idx], rates[idx]))
         real_flow_id = int(self.real_id[idx])
         return_rates[real_flow_id] = rate #rates[idx]
         idx +=1

        
       # check for new flows
       #for f in self.Flows:
       #  if(f.starttime <= it and f.added == False):
           #print("flow start time %d it %d"%(f.starttime, it))
       #    self.addFlow(f, it)

       # remove any ended flows
       #for fidx in range(0, (self.umax_mp.data_sent.shape)[0]):
         ##print("%d %d %d %d" %((self.umax_mp.data_sent.shape)[0], (self.umax_mp.maxdata.shape)[0], fidx, self.umax_mp.num_flows))
        # if(fidx < (self.umax_mp.data_sent.shape)[0]): # double check for multiple deletions in a single loop
        #   if(self.umax_mp.data_sent[fidx] > self.umax_mp.maxdata[fidx]):
        #     self.dropFlow(fidx, it)

       #if self.umax_mp.opt_gap < tol:   
       #    good += 1
       #else:
       #    good = 0
    #return ((good==conseq_good), self.umax_mp.opt_gap, it)
    return return_rates
  

  def main_solver(self,B, w, c, numports, numflows, prices, ratios, data_sent, flow_sizes, flowids):
    nports = numports;
    nflows = numflows
    (A, nflows, w ) = gen_A_from_B(B)

    need_check = []
    alpha=1.0
    
    #for inst in range(num_instances):
    #nports = np.random.randint(3, 50)
    #nflows = np.random.randint(nports/3, 10*nports)
    #(A, w, c) = gen_bipartite_instance(nports, nflows)
    #print ("A = ")
    #print A

     
    self.umax_mp = UtilMax(A, w, c, alpha=alpha, method='mp')
    (success, opt_gap, num_it) = self.umax_mp.solve(tol=1e-10, max_it=1000)
    
    #umax_gradient = UtilMax(A, w, c, alpha=alpha, method='gradient')
    #umax_gradient.solve(tol=1e-10, max_it=1000)
                                      
    #print '\n###################### instance %d #######################' % inst
    print '\n###################### instance #######################'
    print 'nflows=', A.shape[0]
    print 'nports=', A.shape[1]/2
    print 'num_it=', num_it
                
    if success:
        print 'optimality gap=', opt_gap, ' --> good'
    else:                
        need_check.append(0)
        print 'A='
        print A
        print 'w='
        print w.T
        print 'c='
        print c.T
        print '**********************************************************'
                    
        umax_mp.print_details()
        print '---'
        #umax_gradient.print_details()
        
        print 'need_check thus far='
        print need_check
                                                                        
    ##plt.figure('instance %d -- link rate' % inst)
    #plt.figure(1)
    #plt.title('link rates')
    #plt.plot(np.dot(umax_mp.xs.T, umax_mp.routes))
    ##plt.plot(np.dot(umax_gradient.xs.T, umax_mp.routes), '--')
    #plt.legend(range(umax_mp.num_links))
    #plt.show()
                
    ##plt.figure('instance %d -- price' % inst)
    #plt.figure(2)
    #plt.title('link prices')
    #plt.plot(umax_mp.prs.T)
    ##plt.plot(umax_gradient.prs.T, '--')
    #plt.legend(range(umax_mp.num_links))
    #plt.show()  

    ##plt.figure('instance %d -- flow rate' % inst)
    #plt.figure(3)
    #plt.title('flow rates')
    #plt.plot(umax_mp.xs.T)
    ##plt.plot(umax_gradient.xs.T, '--')
    #plt.ylim(0,2)
    #plt.legend(range(umax_mp.num_flows))
    #plt.show()

    ##plt.figure('instance %d -- optimality gap' % inst)
    #plt.figure(4)
    #plt.title('optimality gap')
    #plt.semilogy(umax_mp.opt_gaps)
    ##plt.semilogy(umax_gradient.opt_gaps, '--')
    #plt.show()            
        
    sys.stdout.flush()
                
if __name__ == '__main__':
#    main()
    main_solver()


