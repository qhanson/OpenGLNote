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