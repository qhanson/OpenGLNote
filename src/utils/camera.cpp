#include "camera.hpp"
#include "glfw.hpp"
bool FirstPersonCameraController::update(float elapsedTime)
{
    bool middleButtonPressed =
        glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_MIDDLE);
    if (middleButtonPressed && !m_MiddleButtonPressed)
    {
        m_MiddleButtonPressed = true;
        glfwGetCursorPos(m_pWindow, &m_LastCursorPosition.x, &m_LastCursorPosition.y);
    }

    if (!middleButtonPressed && m_MiddleButtonPressed)
    {
        m_MiddleButtonPressed = false;
    }

    const auto cursorDelta = ([&]()
                              {
    if (m_MiddleButtonPressed)
    {
        glm::dvec2 cursorPosition;
        glfwGetCursorPos(m_pWindow, &cursorPosition.x, &cursorPosition.y);
        const auto delta = cursorPosition - m_LastCursorPosition;
        m_LastCursorPosition = cursorPosition;
        return delta;
    }
    return glm::dvec2(0); })();

    float truckLeft = 0.f;
    float dollyIn = 0.f;
    float pedestalUp = 0.f;
    float rollRightAngle = 0.f;

    if (glfwGetKey(m_pWindow, GLFW_KEY_W))
    {
        dollyIn += m_fSpeed * elapsedTime;
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_S))
    {
        dollyIn -= m_fSpeed * elapsedTime;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_A))
    {
        truckLeft += m_fSpeed * elapsedTime;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_D))
    {
        truckLeft -= m_fSpeed * elapsedTime;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_UP))
    {
        pedestalUp += m_fSpeed * elapsedTime;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_DOWN))
    {
        pedestalUp -= m_fSpeed * elapsedTime;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_Q))
    {
        rollRightAngle -= 0.001f;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_E))
    {
        rollRightAngle += 0.001f;
    }

    const float panLeftAngle = -0.001f * float(cursorDelta.x);
    const float tiltDownAngle = 0.001f * float(cursorDelta.y);
    const auto hasMoved = truckLeft || pedestalUp || dollyIn || panLeftAngle ||
                          tiltDownAngle || rollRightAngle;
    if (!hasMoved)
    {
        return false;
    }
    m_camera.moveLocal(truckLeft, pedestalUp, dollyIn);
    m_camera.rotateLocal(rollRightAngle, tiltDownAngle, 0.f);
    m_camera.rotateWorld(panLeftAngle, m_worldUpAxis);

    return true;
}

bool TrackballCameraController::update(float elapsedTime)
{

    bool middleButtonPressed =
        glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_MIDDLE);
    if (middleButtonPressed && !m_MiddleButtonPressed)
    {
        m_MiddleButtonPressed = true;
        glfwGetCursorPos(m_pWindow, &m_LastCursorPosition.x, &m_LastCursorPosition.y);
    }

    if (!middleButtonPressed && m_MiddleButtonPressed)
    {
        m_MiddleButtonPressed = false;
    }
    const auto cursorDelta = ([&]()
                              {
        if(m_MiddleButtonPressed)
        {
            glm::dvec2 cursorPosition;
            glfwGetCursorPos(m_pWindow, &cursorPosition.x, &cursorPosition.y);
            const auto delta = cursorPosition - m_LastCursorPosition;
            m_LastCursorPosition = cursorPosition;
            return delta;
        }
        return glm::dvec2(0); })();
    if (glfwGetKey(m_pWindow, GLFW_KEY_LEFT_SHIFT))
    {
        const auto truckLeft = 0.01f * float(cursorDelta.x);
        const auto pedestalUp = 0.01f * float(cursorDelta.y);
        const auto hasMoved = truckLeft || pedestalUp;
        if (!hasMoved)
        {
            return false;
        }

        // pan
        m_camera.moveLocal(truckLeft, pedestalUp, 0);

        return true;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_LEFT_CONTROL))
    {
        auto mouseOffset = 0.01f * float(cursorDelta.x);
        if (mouseOffset == 0.f)
        {
            return false;
        }
        // zoom

        const auto viewVector = m_camera.center() - m_camera.eye();

        const auto l = glm::length(viewVector);
        if (mouseOffset > 0.f)
        {
            mouseOffset = glm::min(mouseOffset, l - 1e-4f);
        }

        const auto front = viewVector / l;
        const auto translationVector = mouseOffset * front;

        const auto newEye = m_camera.eye() + translationVector;
        m_camera = CommonCamera(newEye, m_camera.center(), m_worldUpAxis);
        return true;
    }

    const auto longitudeAngle = 0.01f * float(cursorDelta.y); // vertical
    const auto latitudeAngle = -0.01f * float(cursorDelta.x); //  horizitional
    const auto hasMoved = longitudeAngle || latitudeAngle;
    if (!hasMoved)
    {
        return false;
    }
    const auto depthAxis = m_camera.eye() - m_camera.center();
    const auto latitudeRotationMatrix =
        glm::rotate(glm::mat4(1), latitudeAngle, m_worldUpAxis);

    const auto longtitudeRotationMatrix =
        glm::rotate(latitudeRotationMatrix, longitudeAngle, m_camera.left());
    const auto finalDepthAxis = glm::vec3(longtitudeRotationMatrix * glm::vec4(depthAxis, 0));
    const auto newEye = m_camera.center() + finalDepthAxis;

    m_camera = CommonCamera(newEye, m_camera.center(), m_worldUpAxis);
    // rotate around center
    return true;
}
