from gurobipy import Model, GRB, quicksum 
import time
from methods import *

def grb_solve(g:GeometricGraph, devices,timelimit = 600):
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
    
    model=Model()
    x = model.addVars(index_x, vtype=GRB.BINARY, name = "x")
    y = model.addVars(index_y, vtype=GRB.BINARY, name = "y")
    beta = model.addVars(index_x, vtype=GRB.BINARY, name = "beta")
    
    model.setObjective(quicksum(x[i,j,s,t]*costs[i,j] for i,j,s,t in index_x)+1*quicksum(beta[i,j,s,t] for i,j,s,t in index_x))
    model.ModelSense = GRB.MINIMIZE
    for s,t in index_y:
        for i in range(n):
            if i == s:
                model.addConstr(quicksum(x[i,j,s,t] for j in nbh[i]) - quicksum(x[j,i,s,t] for j in nbh[i]) ==  y[s,t])
            elif i == t:
                model.addConstr(quicksum(x[i,j,s,t] for j in nbh[i]) - quicksum(x[j,i,s,t] for j in nbh[i]) == -y[s,t])
            else:
                model.addConstr(quicksum(x[i,j,s,t] for j in nbh[i]) - quicksum(x[j,i,s,t] for j in nbh[i]) == 0)
    
    for i,j,s,t in index_x:
        model.addConstr(quicksum(x[j,k,s,t] for k in nbh[j] if not (g.vertex(k)-g.vertex(j)).IsWeakParallel(g.vertex(i)-g.vertex(j))) + x[i,j,s,t] -1 <= beta[i,j,s,t])

    class ToExplore:
        def __init__(self,ts):
            self.status = {t:False for t in ts}
            self.n = 0
            self.stac = []
            
        def add(self,tp):
            self.stac.append(tp)
            self.n+=1
            self.status[tp]=True
            
    def mycallback(model,where):
        if where == GRB.Callback.MIPSOL:
            ysol = model.cbGetSolution(y)
            look = ToExplore(devices)
            t = devices[0]
            look.add(t)
            while len(look.stac) > 0:
                t = look.stac.pop()
                for tp in devices:
                    if look.status[tp]:
                        continue
                    acti = ysol[t,tp] if t< tp else ysol[tp,t]
                    if acti:
                        look.add(tp)
            if look.n < len(devices):
                ta,tb = [], []
                for t in devices:
                    if look.status[t]:
                        ta.append(t)
                    else:
                        tb.append(t)
                model.cbLazy(quicksum(y[s,t] if s < t else y[t,s] for s in ta for t in tb)>=1)
                        
    model.addConstr(quicksum(y[s,t] for s,t in index_y) == len(devices)-1)
    model.addConstrs((quicksum(y[s,t] if s < t else y[t,s] for t in devices if t != s) >= 1 for s in devices))
    model.Params.lazyConstraints = 1       

    model.Params.TimeLimit = timelimit
    model.Params.MIPGap = 1e-6
    model.setParam("MIPGap",1e-6)
    
    start = time.time()
    model.optimize(mycallback)
    end = time.time()
    
    try:
        print(model.ObjVal)
    except:
        return end-start, None, None, None
        
    allpaths = []
    totalcost = 0.0
    totalbend = 0
    
    try:
        for s,t in index_y:
            if y[s,t].X > 0.7:
                print(f"{s}->{t}, cost {sum(x[i,j,s,t].X*costs[i,j] for i,j in arcs)}, bends {sum(beta[i,j,s,t].X for i,j in arcs)}")
                thispath = [t]
                u = t 
                while u != s:
                    for i in nbh[u]:
                        if x[i,u,s,t].X > 0.7:
                            thispath.append(int(i))
                            u = i
                            break
                thispath.reverse()
                allpaths.append(thispath)
                totalcost += sum(x[i,j,s,t].X*costs[i,j] for i,j in arcs)
                totalbend += sum(beta[i,j,s,t].X for i,j in arcs)
    except:
        return end-start, None, None, None
    
    return end-start, allpaths, totalcost, totalbend
    