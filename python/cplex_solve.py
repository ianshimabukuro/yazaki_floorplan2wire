from docplex.mp.model import Model as cpModel
import cplex
from cplex.callbacks import LazyConstraintCallback
from docplex.mp.callbacks.cb_mixin import *
from methods import *
import time

def cpl_solve(g:GeometricGraph, devices, timelimit=600):
    n = g.num_vertex()
    ne = g.num_edge()
    costs = {}
    nbh = {i:[] for i in range(n)}
    for k in range(ne):
        i = g.edge(k).first
        j = g.edge(k).second
        costs[i,j] = g.weight(k)
        costs[j,i] = g.weight(k)
        nbh[i].append(j)
        nbh[j].append(i)
    arcs = costs.keys()
    
    index_y = [(s,t) for s in devices for t in devices if s<t]
    index_x = [(a[0],a[1],s,t) for a in arcs for s,t in index_y]
    
    model = cpModel()
    x = model.binary_var_dict(index_x)
    y = model.binary_var_dict(index_y,name=lambda key:f"y{key}")
    beta = model.binary_var_dict(index_x)
    
    model.minimize(model.sum(x[i,j,s,t]*costs[i,j] for i,j,s,t in index_x)+model.sum(beta[i,j,s,t] for i,j,s,t in index_x))
    for s,t in index_y:
        for i in range(n):
            rhs = 0
            if i == s:
                rhs = y[s,t]
            elif i == t:
                rhs = -y[s,t]
            model.add_constraint(model.sum(x[i,j,s,t] for j in nbh[i]) - model.sum(x[j,i,s,t] for j in nbh[i]) == rhs)
            
    for i,j,s,t in index_x:
        model.add_constraint(model.sum(x[j,k,s,t] for k in nbh[j] if not (g.vertex(k)-g.vertex(j)).IsWeakParallel(g.vertex(i)-g.vertex(j))) + x[i,j,s,t] -1 <= beta[i,j,s,t])
        
    class ToExplore:
        def __init__(self,ts):
            self.status = {t:False for t in ts}
            self.n = 0
            self.stac = []
            
        def add(self,tp):
            self.stac.append(tp)
            self.n+=1
            self.status[tp]=True
            
    class LazyCut(LazyConstraintCallback):

        def __call__(self):

            look=ToExplore(devices)
            t=devices[0]
            look.add(t)
            ysol = {key:self.get_values(f"y{key}") for key in index_y}
            while len(look.stac) > 0:
                t = look.stac.pop()
                for tp in devices:
                    if look.status[tp]:
                        continue
                    acti = ysol[t,tp] if t< tp else ysol[tp,t]
                    if acti:
                        look.add(tp)
            if look.n < len(devices):
                ta,tb = set(), set()
                for t in devices:
                    if look.status[t]:
                        ta.add(t)
                    else:
                        tb.add(t)
                var=[f"y{key}" for key in index_y if (key[0] in ta and key[1] in tb) or (key[0] in tb and key[1] in ta)]
                self.add(cplex.SparsePair(ind=var,val=[1.0]*len(var)),sense="G",rhs=1)
    
    lazycut=model.register_callback(LazyCut)
    lazycut.y=y
    
    model.add_constraint(model.sum(y[s,t] for s,t in index_y) == len(devices)-1)
    model.add_constraints((model.sum(y[s,t] if s < t else y[t,s] for t in devices if t != s) >= 1 for s in devices))
    
    model.set_log_output(True)
    model.parameters.timelimit = timelimit
    # model.parameters.threads = 4 
    model.parameters.mip.tolerances.mipgap=1e-6
    start=time.time()
    model.solve()
    end=time.time()
        
    allpaths = []
    totalcost = 0.0
    totalbend = 0
    
    try:
        print("Objective value: ", model.solution.get_objective_value())
    except:
        return end-start, None, None, None
    
    try:
    
        for s,t in index_y:
            if y[s,t].solution_value > 0.7:
                print(f"{s}->{t}, cost {sum(x[i,j,s,t].solution_value*costs[i,j] for i,j in arcs)}")
                path = [int(t)]
                u = t 
                while u != s:
                    for v in nbh[u]:
                        if x[v,u,s,t].solution_value > 0.7:
                            u = v
                            path.append(int(u))
                            break
                path.reverse()
                allpaths.append(path)
                totalcost += sum(x[i,j,s,t].solution_value*costs[i,j] for i,j in arcs)
                totalbend += sum(beta[i,j,s,t].solution_value for i,j in arcs)
    except:
        return end-start, None, None, None
    return end-start, allpaths, totalcost, totalbend