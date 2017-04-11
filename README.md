# dxfio
Reads and writes dxf files with a focus on meshes, polylines, and points.

## Building

On Windows, run:

```
mkdir Build && cd Build
cmake .. -G "Visual Studio 14 2015 Win64"
```

Should be similar on Linux, OSX, but without the Visual Studio part.

## Notes

- dxfio only imports Meshes, Polylines, and Points. So, if you using, say, Rhino3D, and you have a bunch of smooth surfaces/curves, you should use the DXF export options "2004 Polylines".
- A Force Y Up option is coming soon.
