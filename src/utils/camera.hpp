#pragma once
#include "glfw.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float YAW = -90.f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw, Pitch;

    float MovementSpeed, MouseSensitivity, Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;

        updateCameraVectors();
    }

    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
           float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;

        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyBoard(CameraMovement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
        {
            Position += Front * velocity;
        }

        if (direction == BACKWARD)
        {
            Position -= Front * velocity;
        }

        if (direction == LEFT)
        {
            Position -= Right * velocity;
        }
        if (direction == RIGHT)
        {
            Position += Right * velocity;
        }
    }

    void ProcessMouseMovement(float xoffset, float yoffset,
                              GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch -= yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
            {
                Pitch = 89.0f;
            }

            if (Pitch < -89.0f)
            {
                Pitch = -89.0f;
            }
        }

        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= yoffset;

        if (Zoom < 1.0f)
        {
            Zoom = 1.0f;
        }

        if (Zoom > 45.0f)
        {
            Zoom = 45.0f;
        }
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

class CommonCamera
{
private:
    glm::vec3 m_eye, m_center, m_up;

public:
    CommonCamera() = default;

    CommonCamera(glm::vec3 e, glm::vec3 c, glm::vec3 u)
        : m_eye(e), m_center(c), m_up(u)
    {
        const auto front = m_center - m_eye;
        const auto left = cross(m_up, front);
        assert(left != glm::vec3(0));
        m_up = normalize(cross(front, left));
    }

    glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(m_eye, m_center, m_up);
    }

    void truckLeft(float offset)
    {
        const auto front = m_center - m_eye;
        const auto left = normalize(cross(m_up, front));
        const auto translationVector = offset * left;
        m_eye += translationVector;
        m_center += translationVector;
    }

    void pedestalUp(float offset)
    {
        const auto translationVector = offset * m_up;
        m_eye += translationVector;
        m_center += translationVector;
    }

    void dollyIn(float offset)
    {
        const auto front = normalize(m_center - m_eye);
        const auto translationVector = offset * front;
        m_eye += translationVector;
        m_center += translationVector;
    }

    void moveLocal(float truckLeftOffset, float pedestalUpOffset, float dollyIn)
    {
        const auto front = normalize(m_center - m_eye);
        const auto left = normalize(cross(m_up, front));
        const auto translationVector =
            truckLeftOffset * left + pedestalUpOffset * m_up + dollyIn * front;
        m_eye += translationVector;
        m_center += translationVector;
    }

    void rollRight(float radians)
    {
        const auto front = m_center - m_eye;
        const auto rollMatrix = glm::rotate(glm::mat4(1), radians, front);
        m_up = glm::vec3(rollMatrix * glm::vec4(m_up, 0.f));
    }

    void tiltDown(float radians)
    {
        const auto front = m_center - m_eye;
        const auto left = cross(m_up, front);
        const auto tiltMatrix = glm::rotate(glm::mat4(1), radians, left);

        const auto newFront = glm::vec3(tiltMatrix * glm::vec4(front, 0.f));
        m_center = m_eye + newFront;
        m_up = glm::vec3(tiltMatrix * glm::vec4(m_up, 0.f));
    }

    void panLeft(float radians)
    {
        const auto front = m_center - m_eye;
        const auto panMatrix = glm::rotate(glm::mat4(1), radians, m_up);
        const auto newFront = glm::vec3(panMatrix * glm::vec4(front, 0.f));
        m_center = m_eye + newFront;
    }

    void rotateLocal(float rollRight, float tiltDown, float panLeft)
    {
        const auto front = m_center - m_eye;
        const auto rollMatrix = glm::rotate(glm::mat4(1), rollRight, front);
        m_up = glm::vec3(rollMatrix * glm::vec4(m_up, 0.f));
        const auto left = cross(m_up, front);
        const auto tiltMatrix = glm::rotate(glm::mat4(1), tiltDown, left);

        const auto newFront = glm::vec3(tiltMatrix * glm::vec4(front, 0.f));
        m_center = m_eye + newFront;
        m_up = glm::vec3(tiltMatrix * glm::vec4(m_up, 0.f));

        const auto panMatrix = glm::rotate(glm::mat4(1), panLeft, m_up);
        const auto newFront2 = glm::vec3(panMatrix * glm::vec4(newFront, 0.f));
        m_center = m_eye + newFront2;
    }

    void rotateWorld(float radians, const glm::vec3 &axis)
    {
        const auto rotationMatrix = glm::rotate(glm::mat4(1), radians, axis);

        const auto front = m_center - m_eye;
        const auto newFront = glm::vec3(rotationMatrix * glm::vec4(front, 0.f));
        m_center = m_eye + newFront;

        m_up = glm::vec3(rotationMatrix * glm::vec4(m_up, 0.f));
    }

    const glm::vec3 eye() const { return m_eye; }
    const glm::vec3 center() const { return m_center; }
    const glm::vec3 up() const { return m_up; }

    const glm::vec3 front(bool normalize = true) const
    {
        const auto f = m_center - m_eye;
        return normalize ? glm::normalize(f) : f;
    }

    const glm::vec3 left(bool normalize = true) const
    {
        const auto f = m_center - m_eye;
        const auto l = cross(m_up, f);
        return normalize ? glm::normalize(l) : l;
    }
};

class CameraController
{
public:
    virtual ~CameraController() {}

    virtual void setCamera(const CommonCamera &camera) = 0;
    virtual const CommonCamera &getCamera() const = 0;
    virtual bool update(float elaspedTime) = 0;
};

class FirstPersonCameraController : public CameraController
{
public:
    FirstPersonCameraController(GLFWwindow *window, float speed = 1.f,
                                const glm::vec3 &worldUpAxis = glm::vec3(0, 1, 0))
        : m_pWindow(window),
          m_fSpeed(speed),
          m_worldUpAxis(worldUpAxis),
          m_camera{glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)}
    {
    }

    void setSpeed(float speed) { m_fSpeed = speed; }
    float getSpeed() const { return m_fSpeed; }

    void increaseSpeed(float delta)
    {
        m_fSpeed += delta;
        m_fSpeed = glm::max(m_fSpeed, 0.f);
    }

    const glm::vec3 &getWorldUpAxis() const { return m_worldUpAxis; }
    void setWorldUpAxis(const glm::vec3 &worldUpAxis)
    {
        m_worldUpAxis = worldUpAxis;
    }

    bool update(float elapsedTime) override;

    const CommonCamera &getCamera() const override { return m_camera; }
    void setCamera(const CommonCamera &camera) override { m_camera = camera; }

private:
    GLFWwindow *m_pWindow = nullptr;
    float m_fSpeed = 0.f;
    glm::vec3 m_worldUpAxis;

    bool m_MiddleButtonPressed = false;
    glm::dvec2 m_LastCursorPosition;

    CommonCamera m_camera;
};

class TrackballCameraController : public CameraController
{
public:
    TrackballCameraController(GLFWwindow *window, float speed = 1.f,
                              const glm::vec3 &worldUpAxis = glm::vec3(0, 1, 0))
        : m_pWindow(window),
          m_fSpeed(speed),
          m_worldUpAxis(worldUpAxis),
          m_camera{glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)}
    {
    }

    void setSpeed(float speed) { m_fSpeed = speed; }
    float getSpeed() const { return m_fSpeed; }

    void increaseSpeed(float delta)
    {
        m_fSpeed += delta;
        m_fSpeed = glm::max(m_fSpeed, 0.f);
    }

    const glm::vec3 &getWorldUpAxis() const { return m_worldUpAxis; }
    void setWorldUpAxis(const glm::vec3 &worldUpAxis)
    {
        m_worldUpAxis = worldUpAxis;
    }

    bool update(float elapsedTime) override;
    const CommonCamera &getCamera() const override { return m_camera; }
    void setCamera(const CommonCamera &camera) override { m_camera = camera; }

private:
    GLFWwindow *m_pWindow = nullptr;
    float m_fSpeed = 0.f;
    glm::vec3 m_worldUpAxis;

    bool m_MiddleButtonPressed = false;
    glm::dvec2 m_LastCursorPosition;
    CommonCamera m_camera;
};