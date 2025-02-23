from methods import *
from visualization import *
import plotly.graph_objects as go
import plotly.colors as pc
from swig_access import *

if __name__ == '__main__':
    
    instanceno = 5
    fileloader = RevitJsonLoader(f"data/realworld/{instanceno}-ElecInfo.json")
    configloader = ConfigLoader(f"data/realworld/{instanceno}-electricitysetting.json")
    # Load information into lists
    
    walls = fileloader.get_walls() # Example, assuming at least one wall exists
    PSB = fileloader.get_PSB()
    devices = fileloader.get_devices()
    doors = fileloader.get_doors()

    #Circuits created by their room ID
    cirs = fileloader.get_devices_per_room()
    #Substitute the circuit in the data pulled from the JSON electricity setting
    configloader.substitute_circuit(cirs)

    circuits = configloader.get_circuits() 
    circuits = sorted(circuits)
    


    # Initialize Visualization 
    fig = go.Figure()
    fig_add_full_structure(fig,walls,PSB,doors,devices)
    x_center,y_center,z_center,max_range = get_axis_boundaries(walls,devices,PSB,doors)
    circuit_colors = pc.qualitative.Plotly
    num_colors = len(circuit_colors)

    for idx, cir in enumerate(circuits):
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
        #print graph 
        
        # Convert GraphConstructor to NetworkX
        G, positions = graphconstructor_to_networkx(gc)

        # Plot in 3D using Plotly
        #plot_3d_network(G, positions)

        print('this is gc.psb',gc.PSB_index)
        # Ours
        da = DecompositionApproach(gc.g) #Wtf is this


        da.PSB = gc.PSB_index
        da.devices = gc.devices_indices
        da.solve(use_mst = True)
        fig_add_paths(fig, gc, da.paths, circuit_colors[idx % num_colors])
        
        print(f'instance {instanceno}, circuit {cir}, devices = {len(devices_subset)+1}, cost = {da.obj.first :.2f}, bend = {da.obj.second}')
        # # Gurobi
        # from gurobi_solve import grb_solve
        # grb_solve(gc.g, [gc.PSB_index]+devices_subset)
            
        # # Cplex
        # from cplex_solve import cpl_solve
        # cpl_solve(gc.g, [gc.PSB_index]+devices_subset)
    

    #Also plotly
    fig.update_layout(
        title="Building Layout Visualization",
        scene=dict(
            xaxis=dict(range=[x_center - max_range / 2, x_center + max_range / 2]),
            yaxis=dict(range=[y_center - max_range / 2, y_center + max_range / 2]),
            zaxis=dict(range=[z_center - max_range / 2, z_center + max_range / 2]),
            aspectmode='manual',
            aspectratio=dict(x=1, y=1, z=1)  # Force 1:1:1 scaling for all axes
        ),
        legend=dict(x=0, y=1)
    )
    
    fig.write_html("building_layout.html")
    # Show the plot
    fig.show()
