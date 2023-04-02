#include "ToyOpenGL.hpp"
#include "utils/shaders.hpp"
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
    GLuint vao = createTriangleVao();
    GLProgram program = compileProgram({m_ShaderRootPath / m_VertexShader, m_ShaderRootPath / m_FragmentShader});
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (!m_GLFWHandle.shouldClose())
    {

        // render
        program.use();
        glBindVertexArray(vao);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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

GLuint ToyOpenGLApp::createTriangleVao()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    float vertices[] = {
        0.5f, 0.5f, 0.0f,   // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3};

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // reset buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}
