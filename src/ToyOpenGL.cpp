#include "ToyOpenGL.hpp"
#include "utils/shaders.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "utils/camera.hpp"
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
    lastX = width / 2.0f;
    lastY = height / 2.0f;
    firstMouse = true;
    lastFrame = 0.f;
    deltaTime = 0.f;
    camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
    glfwSetWindowUserPointer(m_GLFWHandle.window(), this);
    glfwSetKeyCallback(m_GLFWHandle.window(), keycallback);
    glfwSetFramebufferSizeCallback(m_GLFWHandle.window(), framebuffer_size_callback);
    // glfwSetCursorPosCallback(m_GLFWHandle.window(), mouse_callback);
    glfwSetScrollCallback(m_GLFWHandle.window(), scroll_callback);
    // glfwSetInputMode(m_GLFWHandle.window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    printGLVersion();
}

int ToyOpenGLApp::run()
{
    glEnable(GL_DEPTH_TEST);
    GLuint vao = createTriangleVao();
    GLProgram program = compileProgram({m_ShaderRootPath / m_VertexShader, m_ShaderRootPath / m_FragmentShader});
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)};

    std::vector<std::pair<std::string, int>> textureNameId = createTextures();
    float mixValue = .5;
    float zTranslate = -3.0f;
    std::unique_ptr<CameraController> cameraController =
        std::make_unique<TrackballCameraController>(m_GLFWHandle.window());
    while (!m_GLFWHandle.shouldClose())
    {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        cameraController->update(deltaTime);

        glm::mat4 model(1.f), view(1.0f), projection(1.0f);
        projection = glm::perspective(glm::radians(camera.Zoom), 1280.f / 720.f, 0.0001f, 100.0f);
        view = cameraController->getCamera().getViewMatrix();
        // render
        glClearColor(0.5f, 0.5f, 0.5f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        program.use();
        glBindVertexArray(vao);

        glUniformMatrix4fv(program.getUniformLocation("view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(program.getUniformLocation("projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));
        for (int i = 0; i < 10; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(20.0f * i), glm::vec3(.0f, .0f, 1.f));
            int index = 0;
            for (auto tex : textureNameId)
            {
                glActiveTexture(GL_TEXTURE0 + index);
                glBindTexture(GL_TEXTURE_2D, tex.second);
                glUniform1i(program.getUniformLocation(tex.first.c_str()), index++);
            }
            glUniform1f(program.getUniformLocation("mixParam"), mixValue);
            glUniformMatrix4fv(program.getUniformLocation("model"), 1, GL_FALSE,
                               glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        imguiNewFrame();

        ImGui::Begin("GUI Control");
        ImGui::SliderFloat("MixValue", &mixValue, 0.0f, 1.0f);
        ImGui::SliderFloat("zTranslate", &zTranslate, -10.0f, 10.0f);

        static int cameraControllerType = 0;
        const auto cameraControllerTypeChanged =
            ImGui::RadioButton("Trackball", &cameraControllerType, 0) ||
            ImGui::RadioButton("First Person", &cameraControllerType, 1);
        if (cameraControllerTypeChanged)
        {
            const auto currentCamera = cameraController->getCamera();
            if (cameraControllerType == 0)
            {
                cameraController = std::make_unique<TrackballCameraController>(
                    m_GLFWHandle.window());
            }
            else if (cameraControllerType == 1)
            {
                cameraController = std::make_unique<FirstPersonCameraController>(
                    m_GLFWHandle.window());
            }

            cameraController->setCamera(currentCamera);
        }
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
    auto thisobj = static_cast<ToyOpenGLApp *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
        glfwSetWindowShouldClose(window, 1);
    }
}

void ToyOpenGLApp::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void ToyOpenGLApp::mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    auto thisobj = static_cast<ToyOpenGLApp *>(glfwGetWindowUserPointer(window));
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (thisobj->firstMouse)
    {
        thisobj->firstMouse = false;
        thisobj->lastX = xpos;
        thisobj->lastY = ypos;
    }

    float xoffst = xpos - thisobj->lastX;
    float yoffset = ypos - thisobj->lastY;

    thisobj->lastX = xpos;
    thisobj->lastY = ypos;
    thisobj->camera.ProcessMouseMovement(xoffst, yoffset);
}

void ToyOpenGLApp::processInput()
{
    auto thisobj = static_cast<ToyOpenGLApp *>(glfwGetWindowUserPointer(m_GLFWHandle.window()));

    if (glfwGetKey(m_GLFWHandle.window(), GLFW_KEY_W) == GLFW_PRESS)
    {
        thisobj->camera.ProcessKeyBoard(FORWARD, thisobj->deltaTime);
    }

    if (glfwGetKey(m_GLFWHandle.window(), GLFW_KEY_S) == GLFW_PRESS)
    {
        thisobj->camera.ProcessKeyBoard(BACKWARD, thisobj->deltaTime);
    }
    if (glfwGetKey(m_GLFWHandle.window(), GLFW_KEY_A) == GLFW_PRESS)
    {
        thisobj->camera.ProcessKeyBoard(LEFT, thisobj->deltaTime);
    }
    if (glfwGetKey(m_GLFWHandle.window(), GLFW_KEY_D) == GLFW_PRESS)
    {
        thisobj->camera.ProcessKeyBoard(RIGHT, thisobj->deltaTime);
    }
}

void ToyOpenGLApp::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto thisobj = static_cast<ToyOpenGLApp *>(glfwGetWindowUserPointer(window));
    thisobj->camera.ProcessMouseScroll(static_cast<float>(yoffset));
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
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3};

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // GLuint ebo;
    // glGenBuffers(1, &ebo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
        textureNameId.push_back(std::make_pair("texture1", texture));
        glBindTexture(GL_TEXTURE_2D, 0);
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
        textureNameId.push_back(std::make_pair("texture2", texture));
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    stbi_image_free(data2);

    return textureNameId;
}
