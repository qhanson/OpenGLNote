#include "ToyOpenGL.hpp"
#include "utils/shaders.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    std::vector<std::pair<std::string, int>> textureNameId = createTextures();
    float mixValue = .5;
    while (!m_GLFWHandle.shouldClose())
    {

        // render
        program.use();
        for (auto tex : textureNameId)
        {
            glUniform1i(program.getUniformLocation(tex.first.c_str()), tex.second);
        }
        glUniform1f(program.getUniformLocation("mixParam"), mixValue);
        glBindVertexArray(vao);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        imguiNewFrame();

        ImGui::Begin("GUI Control");
        ImGui::SliderFloat("MixValue", &mixValue, 0.0f, 1.0f);

        ImGui::End();

        imguiRenderFrame();

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
        1.0f, 1.0f, 0.f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3};

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // reset buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}

std::vector<std::pair<std::string, int>> ToyOpenGLApp::createTextures()
{
    std::vector<std::pair<std::string, int>> textureNameId;
    int width, height, nChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(
        (m_AppPath.parent_path() / "assets" / "wall.jpg").string().c_str(),
        &width,
        &height,
        &nChannels, 0);
    if (data)
    {
        glActiveTexture(GL_TEXTURE0);
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        textureNameId.push_back(std::make_pair("texture1", 0));
    }

    stbi_image_free(data);

    unsigned char *data2 = stbi_load(
        (m_AppPath.parent_path() / "assets" / "awesomeface.png").string().c_str(),
        &width,
        &height,
        &nChannels, 0);
    if (data2)
    {
        glActiveTexture(GL_TEXTURE1);
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
        textureNameId.push_back(std::make_pair("texture2", 1));
    }

    stbi_image_free(data2);

    return textureNameId;
}
