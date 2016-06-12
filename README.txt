SimpleGallery
--------------------------------------------------------------------------------

Author: Zhen Zhi Lee (zzyzy)
Contact: leezhenzhi@gmail.com

A simple gallery made to be more familiar with OpenGL and GLSL.
This is the continuation of the SimpleScene project.

To begin, open the solution file (.sln) with Visual Studio 2013+ and build it.
All the third party libraries required are packaged together for convenience.

Or, a prebuilt binary is available in either Debug/ or Release/, just execute
it to see the gallery.

Most of the images and models have been modified for my own purposes using
Photoshop and Blender. See the disclaimer below for more information regarding
the use of images and models in this project.

The application should be OpenGL 3.3 core profile compliant IF rendering text
uses freetype instead of glutBitmapCharacters(). See TODO section for more info.

Do note that the application may take some time to startup due to model loading,
refer to the known issues section for more info.

Make sure C++11 is supported.

Libraries used are:
--------------------------------------------------------------------------------
deVIL for image loading
assimp for model loading
glm for vector and matrix maths
glew for OpenGL 3.3 extensions
freeglut for windowing

Features:
--------------------------------------------------------------------------------
01. An art gallery with 9 rooms
02. The middle room is the room with screenshot portraits (loaded from the
    screenshots/* folder)
03. The 8 other rooms have game portraits (screenshots taken from Google Image)
    and they are: Overwatch, Diablo, StarCraft, Tomb Raider, Minecraft, Heroes
    of the Storm, Crysis, Dota
04. Pedestals are placed throughout the scene
05. Ornaments are placed throughout the scene
06. Scene is textured whenever applicable
07. Basic phong shading is available
08. Animations (rotating fans, moving ornaments, moving lights and blinking
    lights)
09. Two spotlights (moving)
10. Translucent floor
11. Night time turns off the "sun" and sets the background to black, day time
    turns on the sun and sets the background to white
12. Help display is available by pressing the 'h' key
13. User interactions include:
      Wireframe mode with a
        'z' - Black blackground with white lines
        'x' - White background with black lines
        'c' - Blue background with yellow lines
      Solid mode that are
        'v' - Colored using basic colors without lighting and textures
        'b' - Colored with material and lighting
        'n' - Textured only without lighting
        'm' - Basic phong shading along with texture
      Point light controls are
        '1' - Toggle light for room 1
        '2' - Toggle light for room 2
        '3' - Toggle light for room 3
        '4' - Toggle light for room 4
        '5' - Toggle light for room 5
        '6' - Toggle light for room 6
        '7' - Toggle light for room 7
        '8' - Toggle light for room 8
        '9' - Toggle light for room 9
      Spot light controls are
        ';' - Toggle spot light 1
        ''' - Toggle spot light 2
      Flash light controls are
        'f' - Toggle flash light
        'j' - Increase intensity
        'k' - Decrease intensity
        'l' - Cycle through R, G or B colors
      Other controls are
        't' - Toggle blending
        'i' - Toggle day/night time
        'o' - Toggle full scene multisample anti aliasing
        'p' - Take a screenshot (they are saved in the screenshots/ folder)
        'h' - Toggle help instructions
        ESC - Quit
14. Camera controls are also available and they are:
      'w', 'a', 's', 'd' for the usual FPS control
      'q' - roll left
      'e' - roll right
      PAGE UP - move up
      PAGE DOWN - move down
      HOME - Zoom in
      END - Zoom out
      Arrow UP - Look up
      Arrow DOWN - Look down
      Arrow LEFT - Look to the left
      Arrow RIGHT - Look to the right
      '0' - Reset the camera to the starting point
15. Screenshots can be taken and they are showin in the middle room of the
    gallery
16. The gallery's floor is reflective, turn on blending to try it
    (only the fan will be reflected though -- see known issues)
17. Fullscene anti aliasing is available, turn it on to try

It is highly recommended to disable the help instruction to improve the fps.

Known issues:
--------------------------------------------------------------------------------
01. Model loading during initialization slow
      This is mainly due to how the application generates the buffers for the
      models. Each mesh in a model uses an individual buffer instead of using
      the same vertex array buffer and apply transformation to every instances
      on the gallery. This is a huge waste of memory and loading time.
02. During night time with blending on, the floor may be missing, not sure if
    this is correct because of the floor's blending with the background of the
    scene.
03. Reflective surface only reflects the fan atm. This is because the fps is
    already very low to render all the objects, as such any more object
    rendering will be infeasible. This requires optimization to the application
    and the models used.
04. Motion blur has yet to be implemented (OpenGL 3.3 core profile removed
    the accumulation buffer)

TODO:
--------------------------------------------------------------------------------
01. Implement freetype to render text instead of using the compatibility profile
02. Be OpenGL 3.3 core profile compliant
03. Further optimization with model loading is required
04. Implement motion blur
05. Code refactoring, tidying and optimization (greatly required)
06. GLSL code may need to be revised
07. Use GLFW/SFML instead of GLUT
08. Directory path to models uses a hacky way, this must be changed

Lessons learnt:
--------------------------------------------------------------------------------
01. In GLSL, light with alpha value makes transparent object opaque because of
    scalar vector multiplication (see shaders/full.frag)
02. In Blender, UV mapped texture will be stretched if the object is scaled.
    To solve this, press CTRL+A to apply the scale and unwrap object again.
    UV coords can also be scaled by pressing A in the UV editor window and
    pressing S to scale it. The texture will be repeated as a result.
03. Blender, how to UV map, design a scene, knife tool, change origin point
    and etc.
04. How to reflect objects using the stencil buffer

Disclaimer:
--------------------------------------------------------------------------------
All the images used for textures and the models in the scene are taken via
Google Images and 3dwarehouse of SketchUp. I do not own them and credits belong
to all the respective authors. This project is open source and for learning
purposes, as such, the images and models used are not intended for commercial
purposes.

Some of the codes are referenced online from learnopengl.com, lighthouse3d.com
and etc. Huge thanks & appreciation to them for making OpenGL better
for everyone.
