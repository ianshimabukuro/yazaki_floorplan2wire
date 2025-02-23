from EWDpy import *
import json 
from os.path import join
from os import listdir
from tqdm import tqdm

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
    
    def get_PSB(self):
        for b in self.data['Item5']:
            if "强电" in b['Name']:
                print('Fetched Electrical Panel Information')
                return Device(str(b['Id']), str(b['Name']), pointxyz(b['Point']), str(b['Host_Id']),str(b['Room_Id']))
        raise ValueError('Could not find Electrical Panel information')

    def get_devices(self):
        devices = []
        devjson = self.data['Item4']
        #add junction boxes
    
        #add regular devices
        for item in tqdm(devjson, desc = 'Processing Devices', unit = 'device'):
            print(str(item['Room_Id']),str(item['Name']))
            devices.append(Device(str(item['Id']), str(item['Name']), pointxyz(item['Point']), str(item['Host_Id']),str(item['Room_Id'])))
        return devices 
    
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
        #Correct, using the name of the device instead of the room
        # Dictionary to map room IDs to names
        room_id_to_name = {room['Id']: room['Name'] for room in rooms}
        #print(devices)
        devices_by_room = {}
        for device in devices:
            room_id = device.room_id
            #print(room_id)
            device_id = device.id

            room_name = device.name
            #print(room_name)

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

