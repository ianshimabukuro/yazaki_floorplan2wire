from methods import *
from plotly_visual import *
from cadquery_visual import *
from excel_export import *

import plotly.graph_objects as go
import plotly.colors as pc

from EWDpy import vecIndex
import cadquery as cq
from cadquery.occ_impl.exporters.assembly import exportAssembly


if __name__ == '__main__':
    show_p_grid = False
    show_p_final = True
    export_cad = True
    export_excel = True

    #Load information from JSON into Python Object
    instanceno = 3
    fileloader = RevitJsonLoader(f"data/realworld/{instanceno}-ElecInfo.json")
    configloader = ConfigLoader(f"data/realworld/{instanceno}-electricitysetting.json")
    
    # Load individual information into lists
    data = fileloader.return_data()
    walls = fileloader.get_walls() 
    PSB = fileloader.get_PSB()
    devices = fileloader.get_devices()
    doors = fileloader.get_doors()
    junction_boxes = fileloader.get_junction_boxes()
    
    if export_cad:
        all_walls = get_cad_walls(data)
        door_cutouts = cut_cad_doors(data)
        all_walls = all_walls.cut(door_cutouts)
        dvs = add_dvs(devices)
        wires = cq.Workplane()

    #Join devices with junction_boxes
    devices += junction_boxes

    #Circuits created by their room ID
    cirs = fileloader.get_devices_per_room(devices)

    #Substitute the circuit in the data pulled from the JSON electricity setting
    configloader.substitute_circuit(cirs)

    #Get circuits from electricity seeting info, which was subsitituted by our own
    circuits = configloader.get_circuits() 
    circuits = sorted(circuits)
    
    #Initialize plotly visual
    if show_p_final or show_p_grid:
        fig = go.Figure()
        fig_add_full_structure(fig,walls,PSB,doors,devices)
        x_center,y_center,z_center,max_range = get_axis_boundaries(walls,devices,PSB,doors)
        circuit_colors = pc.qualitative.Plotly
        num_colors = len(circuit_colors)

    #Initialize cad visual
   
        
    
    if export_excel:
        all_lengths = {}

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
        if export_excel:
            lengths = paths_to_lengths(gc,cir,da.paths)
            all_lengths.update(lengths)
            
        if show_p_final:
            fig_add_paths(fig, gc, da.paths, circuit_colors[idx % num_colors])
        #plotly_show(fig, x_center,y_center,z_center, max_range)
        
        if export_cad:
            wires = add_paths(wires,gc,da.paths)
            
        
        if show_p_grid:
            plot_3d_network(gc, x_center,y_center,z_center,max_range)

        print(f'instance {instanceno}, circuit {cir}, devices = {len(devices_subset)+1}, cost = {da.obj.first :.2f}, bend = {da.obj.second}')
        

    #Export excel
    if export_excel:
        lengths_to_excel(all_lengths)
    
    #Export CAD
    if export_cad:
        jbs = add_jbs(junction_boxes)
        psb_box = cq.Workplane("XY").box(400, 150, 600)
        psb_box = psb_box.translate((PSB.location.x, PSB.location.y, PSB.location.z))
        final = cq.Assembly()
        final.add(psb_box.translate((0, 0, -3300 / 2)).rotate((0, 0, 0), (1, 0, 0), -90), color=cq.Color('Red'))
        final.add(all_walls.rotate((0, 0, 0), (1, 0, 0), -90),color=cq.Color(1, 1, 1, 0.85))
        final.add(jbs.translate((0, 0, -3300 / 2)).rotate((0, 0, 0), (1, 0, 0), -90),color=cq.Color("Red"))
        final.add(dvs.translate((0, 0, -3300 / 2)).rotate((0, 0, 0), (1, 0, 0), -90),color=cq.Color("Green"))
        final.add(wires.translate((0, 0, -3300 / 2)).rotate((0, 0, 0), (1, 0, 0), -90),color=cq.Color("Blue"))   
        exportAssembly(final, 'output/final_model.step')
        #cq.exporters.export(final, 'output/final_model.step')
        #.rotate((0, 0, 0), (1, 0, 0), -90)

    #Export HTML of Plotly
    #fig.write_html("6-building_layout.html")
    
    # Show final plot
    if show_p_final:
        plotly_show(fig, x_center,y_center,z_center, max_range)
    
