from EWDpy import *
import math
import json 
from os.path import join
from os import listdir
from tqdm import tqdm
import numpy as np
import cadquery as cq

def ft2mm(x):
    return x*304.8

def pointxyz(dat)->Point:
    return Point(dat['X'], dat['Y'], dat['Z'])

def explore_dict(d, indent=0):
    """Recursively print the structure of a dictionary and count elements."""
    count = 0  # Initialize the count for this level of recursion

    if isinstance(d, dict):
        for key, value in d.items():
            print("  " * indent + f"- {key} ({type(value).__name__})")
            count += explore_dict(value, indent + 1)  # Recursively count elements
    elif isinstance(d, list):
        print("  " * indent + f"- List with {len(d)} items")
        for item in d:
            count += explore_dict(item, indent + 1)  # Count each item in the list
    else:
        print("  " * indent + f"- {type(d).__name__}")
        count += 1  # Increment count when hitting a base element

    return count

# Example

class RevitJsonLoader(object):
    
    def __init__(self,filename) -> None:
        self.data = self.parse_file(filename)


    #turn JSON into dictionary    
    def parse_file(self,filename): 
        try:
            with open(filename,'r') as f:
                data = json.load(f)
        except:
            with open(filename, 'r', encoding='utf8') as f:
                data = json.load(f)
        self.transform_mm(data)
        #count = explore_dict(data)
        #print(count)
        print("JSON file has been properly loaded!")
        return data
            
        
    def transform_mm(self,data):
        for i in range(len(data["Item1"])):
            for j in range(8):
                for key in ["X","Y","Z"]:
                    data["Item1"][i]["Vertexes"][j][key] = ft2mm(data["Item1"][i]["Vertexes"][j][key])
            for j in range(2):
                for key in ["X","Y","Z"]:
                    data["Item1"][i]["LocationCurve"][j][key] = ft2mm(data["Item1"][i]["LocationCurve"][j][key])
                    
        for i in range(len(data["Item4"])):
            for key in ["X","Y","Z"]:
                data['Item4'][i]['Point'][key] = ft2mm(data['Item4'][i]['Point'][key])
        
        for i in range(len(data["Item5"])):
            for key in ["X","Y","Z"]:
                data['Item5'][i]['Point'][key] = ft2mm(data['Item5'][i]['Point'][key])
            data['Item5'][i]['Width'] = ft2mm(data['Item5'][i]['Width'])
            data['Item5'][i]['Height'] = ft2mm(data['Item5'][i]['Height'])
        
        for i in range(len(data["Item7"])):
            for key in ["X","Y","Z"]:
                data['Item7'][i]['Point'][key] = ft2mm(data['Item7'][i]['Point'][key])
            data['Item7'][i]['Width'] = ft2mm(data['Item7'][i]['Width'])
            data['Item7'][i]['Height'] = ft2mm(data['Item7'][i]['Height'])
            for j in range(4):
                for key in ['X', 'Y','Z']:
                    data['Item7'][i]['Boundary'][j][key] = ft2mm(data['Item7'][i]['Boundary'][j][key])
        
        data['Rest']['Item2']['floorheight'] = ft2mm(data['Rest']['Item2']['floorheight'])
        
    def get_walls(self):
        walljson = self.data['Item1']
        walls = []
        for item in tqdm(walljson, desc="Processing walls", unit="item"):
            name = str(item['Name'])
            p0 = pointxyz(item['Vertexes'][0])
            p1 = pointxyz(item['Vertexes'][1])
            p4 = pointxyz(item['Vertexes'][4])
            ps = pointxyz(item['LocationCurve'][0])
            pe = pointxyz(item['LocationCurve'][1])
            t = BarrierType_WALL
            
            if "梁" in name or "beam" in name.lower():
                t = BarrierType_BEAM
                
            walls.append(
                Wall(name, str(item['Id']), ps, pe, p0.distance(p4), p0.distance(p1), t)
            )
        return walls
    
    def get_cad_walls(self):
        walljson = self.data['Item1']
        
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
                .translate((mid_x, mid_y, mid_z))
                .setColor((1, 1, 1))  # Move to correct position
                  # Apply rotation if needed
            )

            # Union each wall into the main Workplane
            walls = walls.union(wall)

        return walls
    #.rotate((0, 0, 0), (1, 0, 0), 90)
    def cut_cad_doors(self):
        doorjson = self.data['Item7']
        door_cutouts = cq.Workplane("XY") 
        
        for dr in doorjson:
            boundary = dr['Boundary']
            mid_point = dr['Point']
            height = abs(boundary[2]["Z"] - boundary[0]["Z"])
            print(height)
            mid_x, mid_y, mid_z = mid_point["X"], mid_point["Y"], mid_point["Z"] + ((3300 - height)/ 2)
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


    
    def get_PSB(self):
        for b in self.data['Item5']:
            if "强电" in b['Name']:
                print('Fetched Electrical Panel Information')
                return Device(str(b['Id']), str(b['Name']), pointxyz(b['Point']), str(b['Host_Id']),str(b['Room_Id']))
        raise ValueError('Could not find Electrical Panel information')

    def get_devices(self):
        devices = []
        devjson = self.data['Item4']

        #add regular devices
        for item in tqdm(devjson, desc = 'Processing Devices', unit = 'device'):
            #print(str(item['Room_Id']),str(item['Name']))
            devices.append(Device(str(item['Id']), str(item['Name']), pointxyz(item['Point']), str(item['Host_Id']),str(item['Room_Id'])))
        return devices 
    
    def get_junction_boxes(self):
        jb = []
        rooms = self.data['Item6']
        i = 1
        for room in rooms:
            start = room['StartPoint']
            end = room['EndPoint']
            id = room['Id']
            #name = room['Name']
            #area = room['area']
            if start and end:
                all_points = { (p["X"], p["Y"]) for p in start+ end }
                # Extract X and Y coordinates separately
                X_coords = [p[0] for p in all_points]
                Y_coords = [p[1] for p in all_points]

                # Compute centroid
                centroid_X = np.mean(X_coords)
                centroid_Y = np.mean(Y_coords)

                centroid = {"X": 304.8*float(centroid_X), "Y": 304.8*float(centroid_Y), "Z": 3300.0}
                jb.append(Device(str(303030+i), str('Junction Box'), pointxyz(centroid), str("Ceiling"),str(id)))
                i+=1
        return jb


    def get_doors(self):
        doors = []
        doorjson = self.data['Item7']
        walljson = self.data['Item1']
        for item in tqdm(doorjson, desc = "Processing Doors", unit = 'Door'):
            host_id = item['Host_Id']
            wl = None 
            for wallitem in walljson:
                if wallitem['Id'] == host_id:
                    wl = wallitem
                    break
            wp0 = pointxyz(wl['Vertexes'][0])
            wp1 = pointxyz(wl['Vertexes'][1])
            thickness = wp0.distance(wp1)
            p0, p1, p2 = [pointxyz(item['Boundary'][i]) for i in range(3)]
            doors.append(
                Door(str(item['Name']), str(item['Id']), p0, p1, p0.distance(p2), thickness, str(host_id), BarrierType_DOOR)
            )
        return doors
    
    def get_devices_per_room(self, devices):
        
        rooms = self.data['Item6']
        room_id_to_name = {room['Id']: room['Name'] for room in rooms}

        devices_by_room = {}
        for device in devices:

            room_id = device.room_id
            device_id = device.id
            room_name = room_id_to_name[int(room_id)]

            if room_name not in devices_by_room:
                devices_by_room[room_name] = {
                    "objId": [],
                    "wires": {
                        "earth": 2.5,
                        "live": 2.5,
                        "neutral": 2.5
                    }
                }
            devices_by_room[room_name]["objId"].append(device_id)
           
        return devices_by_room


    def get_floor_height(self):
        return self.data['Rest']['Item2']['floorheight']
        
class ConfigLoader(object):
    
    def __init__(self,filename):
        self.data =self.loading(filename)
    
    def loading(self,filename):
        try:
            with open(filename, 'r') as f:
                data = json.load(f)
        except:
            with open(filename, 'r',encoding='utf8') as f:
                data = json.load(f)
        return data 
    
    def substitute_circuit(self,circuits):
        self.data['circuit'] = circuits
    
    def unit_cost(self, key):
        key = str(key)
        return self.data['unitlengthcosts'][key]

    def get_circuits(self):
        return self.data['circuit'].keys()
    
    def get_circuit_devices(self, cir):
        return list(map(str,self.data['circuit'][cir]['objId']))
    
    def get_circuits_config(self, cir):
        c = Config()
        c.offset_door = self.data['offset_door']
        c.connect_threshold = self.data['connect_threshold']
        c.mini_radius = self.data['miniradius']
        circuit = self.data['circuit'][cir]
        c.live_wire_unit_cost = self.unit_cost(circuit['wires']['live'])
        c.neutral_wire_unit_cost = self.unit_cost(circuit['wires']['neutral'])
        c.earth_wire_unit_cost = self.unit_cost(circuit['wires']['earth'])
        c.through_wall_conduit_unit_cost = self.unit_cost('through_wall_conduit')
        c.in_groove_conduit_unit_cost = self.unit_cost('in_groove_conduit')
        c.conduit_unit_cost = self.unit_cost('conduit')
        return c 

def load_dir_case(path):
    d = listdir(path)
    if 'devices.txt' not in d:
        return None
    if 'vertices.txt' not in d:
        return None
    if 'edges.txt' not in d:
        return None 
    
    g = GeometricGraph()
    with open(join(path,'vertices.txt'),'r') as f:
        for line in f:
            pnt = map(float,line.split())
            g.add_vertex_simply(Point(*pnt))
    with open(join(path,'edges.txt'),'r') as f:
        for line in f:
            v1,v2,w = line.split()
            v1,v2 = int(v1),int(v2)
            w = float(w)
            g.add_edge_simply(v1,v2,w)
    with open(join(path,'devices.txt'),'r') as f:
        devices = list(map(int,f.read().split()))
    return g,devices

