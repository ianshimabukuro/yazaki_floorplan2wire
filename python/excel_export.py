import math
import pandas as pd


def paths_to_lengths(gc,cir,paths):

    """
    input: 
        gc:graph instance, 
        cir: circuit name list, 
        paths: list of paths from da instance
    output: 
        dictionary :{circuit name: list of lenghts in that circuit}
    """



    lengths = []
    for i in range(len(paths)):
        path_length = 0
        prev = None
        for v in paths[i]:
            current = gc.vertex(v)  # Get the 3D point of the vertex
            if prev is None:
                prev = current
            else:
                path_length+= math.sqrt(pow(current.x-prev.x,2)+pow(current.y-prev.y,2)+pow(current.z-prev.z,2))
                prev = current

        lengths.append(round(path_length/1000,1))
    lengths.reverse()
    lengths.pop(1)
    return {cir:lengths}


def lengths_to_excel(lengths):
    
    """
    input: 
        lengths: list of lengths for an individual room
    """
    # Determine the maximum number of devices (column count)
    max_devices = max(len(lengths) for lengths in lengths.values())

    # Define column names dynamically
    columns = ["home_run"] + [f"device{i}" for i in range(1, max_devices)]

    # Create DataFrame
    df = pd.DataFrame.from_dict(lengths, orient="index", columns=columns)
    df.index.name = "Circuit"

    # Save to Excel
    df.to_excel('output/wiring_lengths.xlsx')