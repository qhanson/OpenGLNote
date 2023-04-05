# OpenGLNote
This is my code note for learning opengl.
 
1. https://gltf-viewer-tutorial.gitlab.io/
2. https://learnopengl.com

## Creating one Window

Let's write one class to intialize GLFW, OpenGL function pointers, and ImGUI while creating one window.
The class GLFWHandle is non-copyable. 

## Draw Triangle

vertex array object (VAO), vertex buffer object (VBO), element buffer object (EBO)
Shader program, compile, attach, link, use.

## Texture
2D image texture, uniform sample2D, max 16 textures available for one fragment shader.
Mipmap is best used for small fragments with large solution textures. 

## Transformation

The order of the transformation matters, T*R*S is a general order. We can read the matrix from right to left, which is the default executation order. Rotation matrix rotates around the point (0, 0, 0), given the rotation axis.

## Coordinate System

In general, OpenGL is a right-hand coordinate system. However, in normalized coordinate space, it is a left-hand coordinate system. In addition, depth test for near and far is from 0 to 1. Thus, this is should be a left-hand coordinate system. The glm::perspective projection will inverse the depth. More experiments are required to be conducted.