"""Exports a 3D shape composed of multiple curves

Curves are either polylines or quadratic bezier curves (2 points, 2 handles).
If the curve is a polyline, it is split into individual line segments.
Bezier curves MUST be defined by 4 points so that they will be compatible with 
swadge shapes functions.

Curves are exported in a custom plain text format where fields are delimited by white space
<spline_id> <color> <point1> <point2> (<point3> <point4>)
Points 3 and 4 apply only to bezier curves

0 c050 0,0,0 100,100,0
1 c010 0,0,0 0,100,0 100,100,0 100,0,0
"""

import bpy
import mathutils

def format_points(points, color, spline_id):
    """Formats curve points as a single line in the output file
    
    Args:
        points (list(list)): A list of 3D points
        color (str): The color of this curve, matching an enum member
            from palette.h such as "c111"
        spline_id (int): The id of this spline type (0=polyline, 1=bezier)
        
    Returns:
        A formatted string with the spline id, color, and points, terminated by a newline
    """
    
    depth = 0
    num_points = 0
    points_formatted = []
    for i, point in enumerate(points):
        # Flip Y axis to avoid doing transform on swadge
        depth += point[2]
        num_points += 1
        point_list = [c for c in point]
        point_str = [str(int(component)) for component in point_list]
        point_formatted = ",".join(point_str)
        points_formatted.append(point_formatted)
    
    depth /= num_points
    points_joined = " ".join(points_formatted)
    line = f"{spline_id} {color} {points_joined}\n"
    return line, depth

def get_color(mat):
    """Gets a paletteColor_t-formatted color from a material"""
    if not mat.use_nodes:
        return
    
    node_tree = mat.node_tree
    nodes = node_tree.nodes
    
    node_output = nodes["Material Output"]
    node_group = None
    for node in nodes:
        if node.bl_idname == "ShaderNodeGroup":
            node_group = node
            break
        
    if node_group is None:
        return
    
    r = int(node_group.inputs["R"].default_value)
    g = int(node_group.inputs["G"].default_value)
    b = int(node_group.inputs["B"].default_value)
    
    col = f"c{r}{g}{b}"
    return col
    
fp_out = r"C:\Users\Christian\esp\Super-2024-Swadge-FW\main\modes\music\visualizer\shapes\square.shapes"
display_thickness = 0.005

curve_objs = []
for obj in bpy.data.objects:
    if obj.type == "CURVE":
        curve_objs.append(obj)

text_out = []
depth_map = []
for obj in curve_objs:
    
    bpy.ops.object.select_all(action="DESELECT")
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.bevel_mode = "PROFILE"
    obj.data.bevel_depth = display_thickness   
    
    if len(obj.material_slots) > 0:
        mat = obj.material_slots[0].material
        col = get_color(mat)
        if col is None:
            col = "c555"
    elif "color" in obj:
        col = obj["color"]
    else:
        raise Exception(f"Color missing for curve {obj.name}")
        
    curve = obj.data
    mat = obj.matrix_world
    
    # Assign a "spline_id" based on the spline type which will map to an enum in the swadge mode
    for spline in curve.splines:
        spline_type = spline.type
        if spline_type == "POLY":
            spline_id = 0
            is_cyclic = spline.use_cyclic_u
            segments = list(zip(spline.points, spline.points[1:]))
            
            # Create final segment for cyclic (closed) curve
            if is_cyclic:
                segment_close = [spline.points[-1], spline.points[0]]
                segments.append(segment_close)
            
            # Split polyline into individual line segments
            for segment in segments:
                pt1, pt2 = segment
                pt1 = (mat @ pt1.co.xyz).to_tuple()
                pt2 = (mat @ pt2.co.xyz).to_tuple()
                points = [pt1, pt2]
                segment_formatted, depth = format_points(points, col, spline_id)
                text_out.append(segment_formatted)
                depth_map.append(depth)
        
        # Assume all bezier curves are cubic and only have 2 points
        elif spline_type == "BEZIER":
            spline_id = 1
            is_cyclic = spline.use_cyclic_u
            segments = list(zip(spline.bezier_points, spline.bezier_points[1:]))
            
            # Create final segment for cyclic (closed) curve
            if is_cyclic:
                segment_close = [spline.bezier_points[-1], spline.bezier_points[0]]
                segments.append(segment_close)
            
            # Split bezier into individual 2-point curves
            for segment in segments:
                pt_start, pt_end = segment
                pt1 = (mat @ pt_start.co).to_tuple()
                pt2 = (mat @ pt_start.handle_right).to_tuple()
                pt3 = (mat @ pt_end.handle_left).to_tuple()
                pt4 = (mat @ pt_end.co).to_tuple()
                points = [pt1, pt2, pt3, pt4]
                segment_formatted, depth = format_points(points, col, spline_id)
                text_out.append(segment_formatted)
                depth_map.append(depth)
            
        elif spline_type == "NURBS":
            raise Exception("NURBS curves not supported")

# Sort all curves by their average Z-coordinate in ascending order
# Establishes draw order by drawing far curves last
text_sorted = [line for _, line in sorted(zip(depth_map, text_out))]
with open(fp_out, "w") as f:
    f.writelines(text_sorted)