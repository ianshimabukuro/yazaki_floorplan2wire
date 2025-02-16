
import networkx as nx
import plotly.graph_objects as go

def graphconstructor_to_networkx(graph):
    """ Converts a GraphConstructor object into a NetworkX graph with positions. """
    
    G = nx.Graph()

    # Add nodes (vertices)
    positions = {}
    for i in range(graph.num_vertex()):
        pos = (graph.vertex(i).x, graph.vertex(i).y, graph.vertex(i).z)
        G.add_node(i, pos=pos)
        positions[i] = pos

    # Add edges
    for i in range(graph.num_edge()):
        edge = graph.edge(i)
        G.add_edge(edge.first, edge.second)

    return G, positions

def plot_3d_network(G, positions):
    """ Plots a 3D NetworkX graph using Plotly. """
    
    # Extract node positions
    x_nodes = [positions[node][0] for node in G.nodes()]
    y_nodes = [positions[node][1] for node in G.nodes()]
    z_nodes = [positions[node][2] for node in G.nodes()]

    # Extract edges
    edge_x = []
    edge_y = []
    edge_z = []
    
    for edge in G.edges():
        x_vals = [positions[edge[0]][0], positions[edge[1]][0], None]  # None to break the line
        y_vals = [positions[edge[0]][1], positions[edge[1]][1], None]
        z_vals = [positions[edge[0]][2], positions[edge[1]][2], None]

        edge_x.extend(x_vals)
        edge_y.extend(y_vals)
        edge_z.extend(z_vals)

    # Create traces for edges and nodes
    edge_trace = go.Scatter3d(
        x=edge_x, y=edge_y, z=edge_z,
        mode='lines',
        line=dict(color='blue', width=2),
        hoverinfo='none'
    )

    node_trace = go.Scatter3d(
        x=x_nodes, y=y_nodes, z=z_nodes,
        mode='markers',
        marker=dict(size=5, color='red'),
        hoverinfo='text'
    )

    # Create figure
    fig = go.Figure(data=[edge_trace, node_trace])
    fig.update_layout(
        title="3D Hanan Grid Visualization",
        scene=dict(
            xaxis_title="X",
            yaxis_title="Y",
            zaxis_title="Z",
        ),
        margin=dict(l=0, r=0, b=0, t=40),
    )

    fig.show()

def wall_dict(ewd_wall):
    return {
        "name": ewd_wall.get_name(),
        "id": ewd_wall.get_id(),
        "type": ewd_wall.get_type(),
        "length": ewd_wall.get_length(),
        "height": ewd_wall.get_height(),
        "thickness": ewd_wall.get_thickness(),
        "allow_through": ewd_wall.allow_through(),
        
        # Positioning
        "start": (ewd_wall.get_start().x, ewd_wall.get_start().y, ewd_wall.get_start().z),
        "end": (ewd_wall.get_end().x, ewd_wall.get_end().y, ewd_wall.get_end().z),

        # Directional Vectors
        "u_direction": (ewd_wall.get_u().x, ewd_wall.get_u().y, ewd_wall.get_u().z),
        "n_normal": (ewd_wall.get_n().x, ewd_wall.get_n().y, ewd_wall.get_n().z),
        "vertical_direction": (ewd_wall.get_vert_direc().x, ewd_wall.get_vert_direc().y, ewd_wall.get_vert_direc().z),

        # Cuboid Representation
        "cuboid_base": (ewd_wall.cu().get_base().x, ewd_wall.cu().get_base().y, ewd_wall.cu().get_base().z),
        "cuboid_height_vector": (ewd_wall.cu().get_h().x, ewd_wall.cu().get_h().y, ewd_wall.cu().get_h().z),
        "cuboid_length_vector": (ewd_wall.cu().get_l().x, ewd_wall.cu().get_l().y, ewd_wall.cu().get_l().z),
        "cuboid_width_vector": (ewd_wall.cu().get_w().x, ewd_wall.cu().get_w().y, ewd_wall.cu().get_w().z),
    }

def psb_dict(psb_obj):
    return {
        "id": psb_obj.id,
        "name": psb_obj.name,
        "location": (psb_obj.location.x, psb_obj.location.y, psb_obj.location.z),  # Assuming location is a Point object
        "attached_wall_id": psb_obj.attach_wall_id,  # If attach_wall_id is a simple value
    }

def device_dict(device_obj):
    return {
        "id": device_obj.id,
        "name": device_obj.name,
        "location": (device_obj.location.x, device_obj.location.y, device_obj.location.z),  # Assuming location is a Point object
        "attached_wall_id": device_obj.attach_wall_id,  # If attach_wall_id is a simple value
    }

def door_dict(door_obj):
    return {
        # Basic Info
        "id": door_obj.get_id(),
        "name": door_obj.get_name(),
        "type": door_obj.get_type(),
        "host": door_obj.get_host(),  # Likely the wall or structure the door is attached to
        
        # Dimensions
        "length": door_obj.get_length(),
        "height": door_obj.get_height(),
        "thickness": door_obj.get_thickness(),
        
        # Positioning
        "start": (door_obj.get_start().x, door_obj.get_start().y, door_obj.get_start().z),
        "end": (door_obj.get_end().x, door_obj.get_end().y, door_obj.get_end().z),
        
        # Directional Vectors
        "u_direction": (door_obj.get_u().x, door_obj.get_u().y, door_obj.get_u().z),
        "n_normal": (door_obj.get_n().x, door_obj.get_n().y, door_obj.get_n().z),
        "vertical_direction": (door_obj.get_vert_direc().x, door_obj.get_vert_direc().y, door_obj.get_vert_direc().z),
        
        # Bounding Box & Limits
        "upper_left": door_obj.get_ul(),  # Likely upper-left coordinate
        "upper_right": door_obj.get_ur(),  # Likely upper-right coordinate
        "z_low": door_obj.get_zlow(),  # Lower z-bound of the door
        "z_up": door_obj.get_zup(),  # Upper z-bound of the door
        
        # Cuboid Representation
        "cuboid_base": (door_obj.cu().get_base().x, door_obj.cu().get_base().y, door_obj.cu().get_base().z),
        "cuboid_height_vector": (door_obj.cu().get_h().x, door_obj.cu().get_h().y, door_obj.cu().get_h().z),
        "cuboid_length_vector": (door_obj.cu().get_l().x, door_obj.cu().get_l().y, door_obj.cu().get_l().z),
        "cuboid_width_vector": (door_obj.cu().get_w().x, door_obj.cu().get_w().y, door_obj.cu().get_w().z),

        # Boolean Flags
        "allow_through": door_obj.allow_through(),
    }





