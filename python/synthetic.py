from methods import *

if __name__ == '__main__':
    path = "../data/synthetic/2-40-3"
    g,devices = load_dir_case(path)
    da = DecompositionApproach(g)
    da.PSB = devices[0]
    da.devices = vecIndex(devices[1:])
    da.solve()
    print(f'instance {path}, cost = {da.obj.first :.2f}, bend = {da.obj.second}')
    
    # # Gurobi
    # from gurobi_solve import grb_solve
    # grb_solve(g, devices)
        
    # # Cplex
    # from cplex_solve import cpl_solve
    # cpl_solve(g, devices)