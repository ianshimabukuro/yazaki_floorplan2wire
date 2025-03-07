import plotly.graph_objects as go

import plotly.graph_objects as go

def fig_add_walls(fig, walls):
    for wl in walls:
        # Extract wall properties
        start = wl.get_start()
        end = wl.get_end()
        thickness = wl.get_thickness()
        height = wl.get_height()

        # Compute wall direction vectors
        dx = end.x - start.x  # Length along x
        dy = end.y - start.y  # Length along y
        length = (dx**2 + dy**2)**0.5  # Compute wall length
        ux, uy = dx / length, dy / length  # Unit vector along wall
        nx, ny = -uy, ux  # Perpendicular (normal) vector for thickness

        # Half thickness offset
        half_thickness_x = (thickness / 2) * nx
        half_thickness_y = (thickness / 2) * ny

        # Calculate 8 corners of the cuboid
        # Bottom face
        p1 = (start.x - half_thickness_x, start.y - half_thickness_y, start.z)
        p2 = (end.x - half_thickness_x, end.y - half_thickness_y, end.z)
        p3 = (end.x + half_thickness_x, end.y + half_thickness_y, end.z)
        p4 = (start.x + half_thickness_x, start.y + half_thickness_y, start.z)

        # Top face
        p5 = (p1[0], p1[1], p1[2] + height)
        p6 = (p2[0], p2[1], p2[2] + height)
        p7 = (p3[0], p3[1], p3[2] + height)
        p8 = (p4[0], p4[1], p4[2] + height)

        # Combine coordinates
        x = [p1[0], p2[0], p3[0], p4[0], p5[0], p6[0], p7[0], p8[0]]
        y = [p1[1], p2[1], p3[1], p4[1], p5[1], p6[1], p7[1], p8[1]]
        z = [p1[2], p2[2], p3[2], p4[2], p5[2], p6[2], p7[2], p8[2]]

        # Triangles to form 6 faces of the cuboid
        i = [0, 1, 2, 0, 2, 3,  # Bottom face
             4, 5, 6, 4, 6, 7,  # Top face
             0, 4, 7, 0, 7, 3,  # Side face 1
             1, 5, 6, 1, 6, 2,  # Side face 2
             0, 1, 5, 0, 5, 4,  # Front face
             3, 2, 6, 3, 6, 7]  # Back face

        j = [1, 2, 3, 0, 1, 2,  # Bottom face
             5, 6, 7, 4, 5, 6,  # Top face
             4, 7, 3, 0, 3, 7,  # Side face 1
             5, 6, 2, 1, 2, 6,  # Side face 2
             1, 5, 4, 0, 4, 5,  # Front face
             2, 6, 7, 3, 7, 6]  # Back face

        k = [2, 3, 0, 1, 2, 3,  # Bottom face
             6, 7, 4, 5, 6, 7,  # Top face
             7, 3, 0, 4, 0, 3,  # Side face 1
             6, 2, 1, 5, 1, 2,  # Side face 2
             5, 4, 0, 1, 0, 4,  # Front face
             6, 7, 3, 2, 3, 7]  # Back face

        # Add the wall as a cuboid
        fig.add_trace(go.Mesh3d(
            x=x, y=y, z=z,
            i=i, j=j, k=k,
            opacity=0.01,
            color='blue',
            name='Wall'
        ))

def fig_add_PSB(fig,PSB):
    psb = PSB.location
    fig.add_trace(go.Scatter3d(
        x=[psb.x], y=[psb.y], z=[psb.z],
        mode='markers',
        marker=dict(color='red', size=10),
        name='PSB'
    ))
    

def fig_add_devices(fig,devices):
    for dev in devices:
        loc = dev.location
        if dev.name == 'Junction Box':
            fig.add_trace(go.Scatter3d(
                x=[loc.x], y=[loc.y], z=[loc.z],
                mode='markers',
                marker=dict(symbol = 'square', color='green', size=5)
            ))
        else:
            fig.add_trace(go.Scatter3d(
                x=[loc.x], y=[loc.y], z=[loc.z],
                mode='markers',
                marker=dict(color='green', size=3)
            ))


def fig_add_doors(fig,doors):
    # Plot Doors
    for door in doors:
        start = door.get_start()
        end = door.get_end()

        # Door coordinates
        xs = [start.x, end.x]
        ys = [start.y, end.y]
        zs = [start.z, end.z]

        # Add doors as 3D lines
        fig.add_trace(go.Scatter3d(
            x=xs, y=ys, z=zs,
            mode='lines',
            line=dict(color='orange', width=5),
            name='Door'
        ))

def fig_add_paths(fig, gc, paths,color):
    for path in paths:
        x, y, z = [], [], []
        for v in path:
            point = gc.vertex(v)  # Get the 3D point of the vertex
            x.append(point.x)
            y.append(point.y)
            z.append(point.z)

        fig.add_trace(go.Scatter3d(
            x=x, y=y, z=z,
            mode='lines',
            line=dict(color=color, width=3),
            name='Path'
        ))

def get_axis_boundaries(walls, devices, PSB, doors):
    # Get axis ranges
    x_vals = []
    y_vals = []
    z_vals = []

    # Collect all coordinates for walls
    for wl in walls:
        start = wl.get_start()
        end = wl.get_end()
        x_vals.extend([start.x, end.x])
        y_vals.extend([start.y, end.y])
        z_vals.extend([start.z, end.z, start.z + wl.get_height(), end.z + wl.get_height()])

    # Collect all coordinates for devices
    for dev in devices:
        loc = dev.location
        x_vals.append(loc.x)
        y_vals.append(loc.y)
        z_vals.append(loc.z)

    # Collect all coordinates for PSB
    psb = PSB.location
    x_vals.append(psb.x)
    y_vals.append(psb.y)
    z_vals.append(psb.z)

    # Collect all coordinates for doors
    for door in doors:
        start = door.get_start()
        end = door.get_end()
        x_vals.extend([start.x, end.x])
        y_vals.extend([start.y, end.y])
        z_vals.extend([start.z, end.z])

    # Get axis ranges
    x_range = [min(x_vals), max(x_vals)]
    y_range = [min(y_vals), max(y_vals)]
    z_range = [min(z_vals), max(z_vals)]

    # Find the maximum range across all axes
    max_range = max(
        x_range[1] - x_range[0],
        y_range[1] - y_range[0],
        z_range[1] - z_range[0]
    )

    # Calculate centers
    x_center = sum(x_range) / 2
    y_center = sum(y_range) / 2
    z_center = sum(z_range) / 2

    return x_center,y_center,z_center, max_range+1000

def fig_add_full_structure(fig,walls,PSB,doors,devices):
    fig_add_walls(fig, walls)
    fig_add_PSB(fig, PSB)
    fig_add_devices(fig, devices)
    fig_add_doors(fig, doors)



