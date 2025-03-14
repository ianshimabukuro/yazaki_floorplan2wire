from methods import *
from plotly_visual import *
from cadquery_visual import *
from excel_export import *

import plotly.graph_objects as go
import plotly.colors as pc

from EWDpy import vecIndex
import cadquery as cq


if __name__ == '__main__':
    
    #Load information from JSON into Python Object
    instanceno = 6
    fileloader = RevitJsonLoader(f"data/realworld/{instanceno}-ElecInfo.json")
    configloader = ConfigLoader(f"data/realworld/{instanceno}-electricitysetting.json")
    
    # Load individual information into lists
    data = fileloader.return_data()
    walls = fileloader.get_walls() 
    PSB = fileloader.get_PSB()
    devices = fileloader.get_devices()
    doors = fileloader.get_doors()
    junction_boxes = fileloader.get_junction_boxes()
    devices += junction_boxes

    #Circuits created by their room ID
    cirs = fileloader.get_devices_per_room(devices)

    #Substitute the circuit in the data pulled from the JSON electricity setting
    configloader.substitute_circuit(cirs)

    #Get circuits from electricity seeting info, which was subsitituted by our own
    circuits = configloader.get_circuits() 
    circuits = sorted(circuits)
    
    #Initialize plotly visual
    fig = go.Figure()
    fig_add_full_structure(fig,walls,PSB,doors,devices)
    x_center,y_center,z_center,max_range = get_axis_boundaries(walls,devices,PSB,doors)
    circuit_colors = pc.qualitative.Plotly
    num_colors = len(circuit_colors)

    #Initialize cad visual
    all_walls = get_cad_walls(data)
    door_cutouts = cut_cad_doors(data)
    all_walls = all_walls.cut(door_cutouts)
   
   
    all_lengths = {}
    wires = cq.Workplane()

    for idx, cir in enumerate(circuits):

        #List of devices id in the current circuit
        devices_id = configloader.get_circuit_devices(cir)
        
        #Config object like costs, thresholds, wire unit cost.
        config = configloader.get_circuits_config(cir)
        
        #Set floor height
        config.floor_height = fileloader.get_floor_height()

        #List of devices object from that list of ids
        devices_subset = [dev for dev in devices if dev.id in devices_id]
        
        #Create constructor
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
    
        #Create Room Harness
        da = DecompositionApproach(gc.g) 
        da.PSB = gc.JB_index
        da.devices = gc.devices_indices
        da.solve(use_mst = False)
        
        #Create Home Run Wires
        da.PSB = gc.PSB_index 
        da.devices = vecIndex()  # Create an empty C++ vector
        da.devices.append(gc.JB_index)  # Add the JB_index as the only element
        da.solve(use_mst = False)



        #Routine Done, Gather important info
        lengths = paths_to_lengths(gc,cir,da.paths)
        all_lengths.update(lengths)
        
        fig_add_paths(fig, gc, da.paths, circuit_colors[idx % num_colors])
        #plotly_show(fig, x_center,y_center,z_center, max_range)
        
        wires = add_paths(wires,gc,da.paths)
        #plot_3d_network(gc)

        print(f'instance {instanceno}, circuit {cir}, devices = {len(devices_subset)+1}, cost = {da.obj.first :.2f}, bend = {da.obj.second}')
        print(lengths)
        

    #Export excel
    lengths_to_excel(all_lengths)
    
    #Export CAD
    cq.exporters.export(all_walls.rotate((0, 0, 0), (1, 0, 0), 90), 'walls_model.step')
    cq.exporters.export(wires, 'wires.step')


    #all_walls=all_walls.rotate((0, 0, 0), (1, 0, 0), 180)

    # Move wires down by half of the wall height (3300 / 2)
    wires = wires.translate((0, 0, -3300 / 2))  
    final_model = all_walls.add(wires)
    cq.exporters.export(final_model.rotate((0, 0, 0), (1, 0, 0), -90), 'final_model.step')

    #Export HTML of Plotly
    #fig.write_html("6-building_layout.html")
    
    # Show final plot
    plotly_show(fig, x_center,y_center,z_center, max_range)
    
