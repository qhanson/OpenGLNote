#pragma once

#include "utils/GLFWHandle.hpp"
#include "utils/filesystem.hpp"
#include "utils/camera.hpp"

class ToyOpenGLApp
{
public:
    ToyOpenGLApp(const fs::path &appPath, uint32_t width, uint32_t height,
                 const std::string &vertexShader, const std::string &fragmentShader,
                 const fs::path &output);

    int run();

private:
    GLsizei m_nWindowWidth = 1280;
    GLsizei m_nWindowHeight = 720;
    float lastX, lastY;
    bool firstMouse;
    float lastFrame, deltaTime;
    Camera camera;
    const fs::path m_AppPath;
    const std::string m_AppName;
    const fs::path m_ShaderRootPath;

    std::string m_VertexShader = "forward.vs.glsl";
    std::string m_FragmentShader = "phong.fs.glsl";

    fs::path m_OutputPath;

    const std::string m_ImGuiIniFilename;
    GLFWHandle m_GLFWHandle{
        int(m_nWindowWidth), int(m_nWindowHeight),
        "ToyOpenGLApp",
        m_OutputPath.empty()};
    void processInput();
    static void keycallback(
        GLFWwindow *window, int key, int scancode, int action, int mods);
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    GLuint createTriangleVao();
    std::vector<std::pair<std::string, int>> createTextures();
};