import plotly.graph_objects as go

def visualize_building_layout(gc,walls, PSB, devices, doors):
    # Initialize 3D Plot
    fig = go.Figure()

    # Plot Walls
    for wl in walls:
        start = wl.get_start()
        end = wl.get_end()

        # Wall coordinates
        xs = [start.x, end.x]
        ys = [start.y, end.y]
        zs = [start.z, end.z]

        # Add walls as 3D lines
        fig.add_trace(go.Scatter3d(
            x=xs, y=ys, z=zs,
            mode='lines',
            line=dict(color='blue', width=5),
            name='Wall'
        ))

    # Plot PSB
    psb = PSB.location
    fig.add_trace(go.Scatter3d(
        x=[psb.x], y=[psb.y], z=[psb.z],
        mode='markers',
        marker=dict(color='red', size=10),
        name='PSB'
    ))

    # Plot Devices
    for dev in devices:
        loc = dev.location
        fig.add_trace(go.Scatter3d(
            x=[loc.x], y=[loc.y], z=[loc.z],
            mode='markers',
            marker=dict(color='green', size=8),
            name='Device'
        ))

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
    # Plot Graph Edges
    for edge in range(gc.num_edge()):
        v1 = gc.vertex(gc.edge(edge).first)
        v2 = gc.vertex(gc.edge(edge).second)
        fig.add_trace(go.Scatter3d(
            x=[v1.x, v2.x],
            y=[v1.y, v2.y],
            z=[v1.z, v2.z],
            mode='lines',
            line=dict(color='gray', width=2),
            name='Edge'
        ))

    # Customize Layout
    fig.update_layout(
        title="Building Layout Visualization",
        scene=dict(
            xaxis_title="X-axis",
            yaxis_title="Y-axis",
            zaxis_title="Z-axis"
        ),
        legend=dict(
            x=0, y=1
        )
    )

    # Show the plot
    fig.show()
