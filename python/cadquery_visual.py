import cadquery as cq
import math


def cut_cad_doors(data):
        doorjson = data['Item7']
        door_cutouts = cq.Workplane("XY") 
        
        for dr in doorjson:
            boundary = dr['Boundary']
            mid_point = dr['Point']
            height = abs(boundary[2]["Z"] - boundary[0]["Z"])
            mid_x, mid_y, mid_z = mid_point["X"], mid_point["Y"], mid_point["Z"] - ((3300 - height)/ 2)
             # Determine orientation
            rotation_angle = 0
            # Determine width based on orientation
            if int(boundary[0]["Y"]) == int(boundary[1]["Y"]):  # Horizontal Door
                width = abs(boundary[1]["X"] - boundary[0]["X"])
                rotation_angle = 0  # No rotation needed
            else:  # Vertical Door
                width = abs(boundary[1]["Y"] - boundary[0]["Y"])
                rotation_angle = 90  # Rotate 90 degrees
            door_cutout = (
                cq.Workplane("XY")
                .box(width, 200, height)
                .rotate((0, 0, 0), (0, 0, 1), rotation_angle)
                .translate((mid_x, mid_y, mid_z))
            )
            door_cutouts = door_cutouts.union(door_cutout)
        return door_cutouts

def get_cad_walls(data):
        walljson = data['Item1']
        
        # Global Workplane to store all walls
        walls = cq.Workplane("XY")  
        
        for wl in walljson:
            rotation_angle = 0
            # Extract start and end points
            point1, point2 = wl['LocationCurve'][0], wl['LocationCurve'][1]

            # Hardcoded dimensions
            length = math.sqrt((point2["X"] - point1["X"])**2 + (point2["Y"] - point1["Y"])**2)
            thickness = 200  # Hardcoded thickness
            height = 3300  # Hardcoded height

            # Compute position
            mid_x = (point1["X"] + point2["X"]) / 2
            mid_y = (point1["Y"] + point2["Y"]) / 2
            mid_z = (point1["Z"] + point2["Z"]) / 2

            # Determine orientation
            if int(point1["Y"]) == int(point2["Y"]):  # Horizontal Wall
                rotation_angle = 0  # No rotation needed
            elif int(point1["X"]) == int(point2["X"]):  # Vertical Wall
                rotation_angle = 90  # Rotate 90 degrees
            # Create individual wall from a new Workplane
            wall = (
                cq.Workplane("XY")
                .box(length, thickness, height)
                .rotate((0, 0, 0), (0, 0, 1), rotation_angle)  # Use hardcoded thickness and height
                .translate((mid_x, mid_y, mid_z)) # Move to correct position
                  # Apply rotation if needed
            )

            # Union each wall into the main Workplane
            walls = walls.union(wall)
        return walls

def add_paths(obj, gc, paths):
    for i in range(len(paths)):
        if not paths[i]:  
            continue
        points = [gc.vertex(v) for v in paths[i]]  # Get the 3D points from vertices
        cq_points = [cq.Vector(p.x, p.y, p.z) for p in points]  # Convert to CadQuery vectors
        
        wire = cq.Wire.makePolygon(cq_points, close = False)  # Create a polyline (path)
        try:
            obj = obj.add(wire)  # Add path to the object
        except ValueError:
            obj = wire
    
    return obj

def add_jbs(junction_boxes):
    jb_size = (200, 200, 100)  
    jbs = cq.Workplane()
    for junction_box in junction_boxes:
        jb = cq.Workplane("XY").box(*jb_size)
        jb = jb.translate((junction_box.location.x, junction_box.location.y, junction_box.location.z))
        jbs.add(jb)
    
    return jbs  # Return the updated assembly

def add_dvs(junction_boxes):
    jb_size = (50, 20, 80)  
    jbs = cq.Workplane()
    for junction_box in junction_boxes:
        jb = cq.Workplane("XY").box(*jb_size)
        jb = jb.translate((junction_box.location.x, junction_box.location.y, junction_box.location.z))
        jbs.add(jb)
    
    return jbs  # Return the updated assembly
    
     