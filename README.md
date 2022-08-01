
## Light System base on OpenGl
----
A simple demo about some basic light base on OpenGL.

### Includes:
* Blinn-Phong model.
* Multiple light sources:
    * Theory Light: Directional, Point, Spot
    * Area Light: Rect, Disk, Circle, Cylinder
    * IBL (See [my final project about PBR on CS6610](https://github.com/YosefQiu/CG_Final))
    * Volumetric Light
* Updated and rebuild the old OpenGL render framework

### Third-party libraries
* **GLFW** (GLFW provides a simple API for creating windows, contexts, and surfaces, receiving input and events.) 
* **GLAD** (GLAD is a Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator based on the official specs.) 
* **ImGui** (ImGui is a bloat-free graphical user interface library for C++.) 
* **GLM** (OpenGL Mathematics (GLM) is a header-only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications. 
* **stb_image** (Decoding different type images like JPG, PNG) 
* **ASSIMP** (A library to import and export various 3d-model-formats including scene-post-processing to generate missing render data.) 

### Some Reslut:
![img](https://cdn.jsdelivr.net/gh/YosefQiuImg/Img@master/uPic/shadow.PNG)
 ![img](https://cdn.jsdelivr.net/gh/YosefQiuImg/Img@master/uPic/arealight.png)
  ![img](https://cdn.jsdelivr.net/gh/YosefQiuImg/Img@master/uPic/ibl.png).
  ![img](https://cdn.jsdelivr.net/gh/YosefQiuImg/Img@master/uPic/volumetric.gif)
  
### TODO:
  * Another way to render area light on Real-Time Rendering. Chapter 10. Local illumination
  * Better shadow optimization.


