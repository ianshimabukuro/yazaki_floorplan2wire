from methods import *
from visualization import *

if __name__ == '__main__':
    instanceno = 0
    fileloader = RevitJsonLoader(f"../data/realworld/{instanceno}-ElecInfo.json")
    configloader = ConfigLoader(f"../data/realworld/{instanceno}-electricitysetting.json")
    walls = fileloader.get_walls()
    PSB = fileloader.get_PSB()
    devices = fileloader.get_devices()
    doors = fileloader.get_doors()
    circuits = configloader.get_circuits()
    circuits = sorted(circuits)
    for cir in circuits:
        devices_id = configloader.get_circuit_devices(cir)
        config = configloader.get_circuits_config(cir)
        config.floor_height = fileloader.get_floor_height()
        devices_subset = [dev for dev in devices if dev.id in devices_id]
        
        gc = GraphConstructor()
        for wl in walls:
            gc.add_wall(wl)
        gc.set_PSB(PSB)
        for door in doors:
            gc.add_door(door)
        for dev in devices_subset:
            gc.add_device(dev)
        gc.read_config(config)
        gc.construct() 
        
        # Ours
        da = DecompositionApproach(gc.g)
        da.PSB = gc.PSB_index
        da.devices = gc.devices_indices
        da.solve()
        #print(f'instance {instanceno}, circuit {cir}, devices = {len(devices_subset)+1}, cost = {da.obj.first :.2f}, bend = {da.obj.second}')
        #plot_house_layout(gc)
        #python realworld.py
        
        

    

        # # Gurobi
        # from gurobi_solve import grb_solve
        # grb_solve(gc.g, [gc.PSB_index]+devices_subset)
            
        # # Cplex
        # from cplex_solve import cpl_solve
        # cpl_solve(gc.g, [gc.PSB_index]+devices_subset)
    visualize_building_layout(gc, walls,PSB,devices,doors)
    
    
