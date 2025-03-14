import cadquery as cq

from cadquery import exporters  # Import the exporters module

def create_room_walls(points, wall_height, wall_thickness=200):
    """
    Create a room using four points and extrude walls up to the given height.
    
    :param points: List of (x, y) tuples defining the room corners.
    :param wall_height: Height of the walls in mm.
    :param wall_thickness: Thickness of the walls in mm (default: 200 mm).
    """
    walls = cq.Workplane("XY")
    
    for i in range(len(points)):
        p1 = points[i]
        p2 = points[(i + 1) % len(points)]  # Loop back to the first point for the last wall
        
        # Calculate wall midpoint and length
        mid_x, mid_y = (p1[0] + p2[0]) / 2, (p1[1] + p2[1]) / 2
        wall_length = ((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2)**0.5
        
        # Create wall as a thin rectangle and extrude
        wall = (
            cq.Workplane("XY")
            .rect(wall_length, wall_thickness)
            .extrude(wall_height)
            .translate((mid_x, mid_y, 0))  # Move to correct position
        )
        
        walls = walls.union(wall)
    
    return walls

# Example room corners
points = [(0, 0), (4000, 0), (4000, 3000), (0, 3000)]  # 4m x 3m room
floor_height = 2500  # 2.5 meters

# Generate walls
room = create_room_walls(points, floor_height)

# Export to STEP file
cq.exporters.export(room, "room.step")

print("Room STEP file saved!")

