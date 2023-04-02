#include "ToyOpenGL.hpp"

ToyOpenGLApp::ToyOpenGLApp(const fs::path &appPath, uint32_t width,
                           uint32_t height, const std::string &vertexShader,
                           const std::string &fragmentShader, const fs::path &output)
    : m_nWindowWidth(width),
      m_nWindowHeight(height),
      m_AppPath{appPath},
      m_AppName{m_AppPath.stem().string()},
      m_ImGuiIniFilename{m_AppName + ".imgui.ini"},
      m_ShaderRootPath{m_AppPath.parent_path() / "shaders"},
      m_OutputPath{output}
{
    if (!vertexShader.empty())
    {
        m_VertexShader = vertexShader;
    }
    if (!fragmentShader.empty())
    {
        m_FragmentShader = fragmentShader;
    }
    glfwSetWindowUserPointer(m_GLFWHandle.window(), this);
    glfwSetKeyCallback(m_GLFWHandle.window(), keycallback);
    glfwSetFramebufferSizeCallback(m_GLFWHandle.window(), framebuffer_size_callback);
    printGLVersion();
}

int ToyOpenGLApp::run()
{
    while (!m_GLFWHandle.shouldClose())
    {

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwPollEvents();
        m_GLFWHandle.swapBuffers();
    }
    return 0;
}

void ToyOpenGLApp::keycallback(
    GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
        glfwSetWindowShouldClose(window, 1);
    }
}

void ToyOpenGLApp::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
