# No Time For Caution

Author: Harrison Chen

Design: 800km above our head, a space station is spinning out of control while orbiting Earth in 27500 km/h. Dock your spacecraft to it first, so that we can stabilize it. **NO TIME FOR CAUTION**.

Screen Shot:

![Screen Shot](screenshot.png)

How Your Asset Pipeline Works:

1. Models and scene data are read by the provided python scripts from Blender project file to .pnct and .scene.
2. Textures are processed by `pack_textures.py` from .jpg to uncompressed binary data (`NTFC.tex`).
3. Meshes and scene are loaded in a normal manner. One another shader is written to utilize the textures to render Earth in a dynamic alternation of day and night

**NOTE:** To successfully execute the modified makefile, you will need Python modules **numpy** and **Pillow**, which have been listed in `scenes/requirements.txt`.

How To Play:

- W/S controls moving forward/backward
- A/D moves left/right
- Q/E rotates counterclockwise/clockwise
- Space/Ctrl moves up/down
- Your goal is getting close to the space station while maintain relatively still to it

Sources: 
- Earth-related realistic textures come from [Solar Textures](https://www.solarsystemscope.com/textures/) under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), based on NASA elevation and imagery data [in public domain](https://www.nasa.gov/multimedia/guidelines/index.html).
- The space station model is adapted from
  - [Low poly space scene](https://www.blendswap.com/blend/19262) authored by [artturi](https://www.blendswap.com/profile/341146) under [CC-BY](https://creativecommons.org/licenses/by/4.0/) license
  - [Modular Industrial Pipes 01](https://polyhaven.com/a/modular_industrial_pipes_01) authored by [Jorge Carmaco](https://www.artstation.com/jorgeandrespinedac) under [CC0](https://creativecommons.org/publicdomain/zero/1.0/) license, which is effectively in public domain
- Thanks to [Christopher Fraser](https://www.youtube.com/channel/UCRlED3y4PPk8jxnvQ1pb1ZQ) for the inspiration of [realistic Earth rendering in Blender](https://christopherfraser.gumroad.com/l/planetshader), but neither code nor data from his work is used.

This game was built with [NEST](NEST.md).

