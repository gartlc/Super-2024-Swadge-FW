# Swadge Visualizer

## Description

The swadge visualizer is a real-time music visualizer. The primary component of the visualizer is a custom ".shapes" file which contains information for rendering a vector graphic.

## Adding Content

### The .shapes file
The .shapes file format is a text file where each row corresponds to a single curve. A curve can either be a line segment or a cubic bezier curve. The first two fields define the curve type and color of the curve. Sample file of a square, centered at (0, 0, -100) and 100x100 units in size:

```
0 c050 -50,-50,-100 -50,50,-100
0 c050 -50,50,-100 50,50,-100
0 c050 50,-50,-100 -50,-50,-100
0 c050 50,50,-100 50,-50,-100
```

- Field 1: 0 (line) or 1 (bezier)
- Field 2: Web-safe color in "cRGB" format, corresponding to an entry in the [paletteColor_t](../../../../components/hdw-tft/include/palette.h) enum
- Field 3/4/..: Coordinates as integers in XYZ order. For a line (curve type 0) there are 2 points, and for a bezier curve (type 1) there are 4 points. For the bezier curve, the 1st and 4th points are the start and end points, whereas the middle 2 points are curve handles

When modeling an object in this format, it is recommended to treat the Blender world coordinate frame as the camera coordinate frame. A "flat" object would be modeled entirely on the XY plane with no Z (height/depth). This will minimize the number of transformations you have to do and produce fewer compounding precision errors. Curves should either be modeled using a "POLY" curve (polyline) or "BEZIER" curve. "NURBS" is not supported. Compound curves (curves with more than 2 points) are automatically separated when exporting.

### Adding colors

Ensure each curve has a "color" property which matches the formatting detailed in field 2 above. You may assign a color to your selected objects in Blender using a small helper function, for example:

```python
import bpy

def set_col(col: str="c020") -> None:
    """Sets a custom color to a selected object"""
    for obj in bpy.context.selected_objects:
        obj["color"] = col

set_col(col="c555")
```

Alternatively, you can assign a color to a custom material consisting of a single Material Output node and a node group. The node group should have 3 named inputs: "R", "G", and "B" (case-sensitive). See the [included sample file](./blender_assets/square.blend) for an example.

### Exporting a .shapes file

Export your curves to a .shapes file using the [included script](./blender_assets/make_shapes.py). This script should be run from the Scripting tab in Blender. You may also run the script in headless mode if you prefer using a command like:

```
blender --background square.blend --python shapes_exporter.py
```