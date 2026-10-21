#include "Camera.hpp"

// load glfw after glad
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

const static float STEP_SCALE = 0.1F;
const static int MARGIN = 100;

Camera::Camera(int WindowWidth, int WindowHeight, float fov, float znear,
               float zfar)
    : m_windowWidth(WindowWidth),
      m_windowHeight(WindowHeight),
      FOV(fov),
      zNear(znear),
      zFar(zfar) {
    m_pos = glm::vec3(0.0F, 0.0F, 0.0F);
    m_target = glm::vec3(0.0F, 0.0F, -1.0F);
    m_target = glm::normalize(m_target);
    m_up = glm::vec3(0.0F, 1.0F, 0.0F);

    Init();
}

Camera::Camera(int WindowWidth, int WindowHeight, float fov, float znear,
               float zfar, const glm::vec3& Pos, const glm::vec3& Target,
               const glm::vec3& Up)
    : m_windowWidth(WindowWidth),
      m_windowHeight(WindowHeight),
      m_pos(Pos),
      m_target(Target),
      m_up(Up),
      FOV(fov),
      zNear(znear),
      zFar(zfar) {
    m_target = glm::normalize(m_target);

    m_up = glm::normalize(m_up);

    Init();
}

void Camera::Init() {
    // Zero yaw faces -Z; positive yaw turns toward +X.
    m_AngleH = glm::degrees(std::atan2(m_target.x, -m_target.z));
    m_AngleV = glm::degrees(std::asin(std::clamp(m_target.y, -1.0f, 1.0f)));

    m_OnUpperEdge = false;
    m_OnLowerEdge = false;
    m_OnLeftEdge = false;
    m_OnRightEdge = false;
    m_mousePos.x = m_windowWidth / 2;
    m_mousePos.y = m_windowHeight / 2;
    // glfwGetCursorPos(&m_mousePos.x, &m_mousePos.y);
    // glutWarpPointer(m_mousePos.x, m_mousePos.y);
}

bool Camera::OnKeyboard(char Key) {
    bool Ret = false;

    switch (Key) {
        case GLFW_KEY_W: {
            m_pos += (m_target * STEP_SCALE);
            Ret = true;
        } break;

        case GLFW_KEY_S: {
            m_pos -= (m_target * STEP_SCALE);
            Ret = true;
        } break;

        case GLFW_KEY_A: {
            glm::vec3 Left = glm::normalize(glm::cross(m_up, m_target));
            Left *= STEP_SCALE;
            m_pos += Left;
            Ret = true;
        } break;

        case GLFW_KEY_D: {
            glm::vec3 Right = glm::normalize(glm::cross(m_target, m_up));
            Right *= STEP_SCALE;
            m_pos += Right;
            Ret = true;
        } break;
    }

    return Ret;
}

void Camera::OnMouse(int x, int y) {
    const int DeltaX = x - m_mousePos.x;
    const int DeltaY = y - m_mousePos.y;
    // int DeltaX=x;
    // int DeltaY=y;

    m_mousePos.x = x;
    m_mousePos.y = y;

    m_AngleH += static_cast<float>(DeltaX) / 20.0F;
    m_AngleV -= static_cast<float>(DeltaY) / 20.0F;
    m_AngleV = std::clamp(m_AngleV, -89.0f, 89.0f);

    if (DeltaX == 0) {
        if (x <= MARGIN) {
            // m_AngleH -= 1.0f;
            m_OnLeftEdge = true;
        } else if (x >= (m_windowWidth - MARGIN)) {
            // m_AngleH += 1.0f;
            m_OnRightEdge = true;
        }
    } else {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (DeltaY == 0) {
        if (y <= MARGIN) {
            m_OnUpperEdge = true;
        } else if (y >= (m_windowHeight - MARGIN)) {
            m_OnLowerEdge = true;
        }
    } else {
        m_OnUpperEdge = false;
        m_OnLowerEdge = false;
    }

    Update();
}

void Camera::OnRender() {
    bool ShouldUpdate = false;

    if (m_OnLeftEdge) {
        m_AngleH -= 0.1F;
        ShouldUpdate = true;
    } else if (m_OnRightEdge) {
        m_AngleH += 0.1F;
        ShouldUpdate = true;
    }

    if (m_OnUpperEdge) {
        if (m_AngleV < 89.0F) {
            m_AngleV = std::min(89.0f, m_AngleV + 0.1F);
            ShouldUpdate = true;
        }
    } else if (m_OnLowerEdge) {
        if (m_AngleV > -89.0F) {
            m_AngleV = std::max(-89.0f, m_AngleV - 0.1F);
            ShouldUpdate = true;
        }
    }

    if (ShouldUpdate) {
        Update();
    }
}

void Camera::Update() {
    const float yaw = glm::radians(m_AngleH);
    const float pitch = glm::radians(m_AngleV);
    m_target = glm::vec3(std::sin(yaw) * std::cos(pitch), std::sin(pitch),
                        -std::cos(yaw) * std::cos(pitch));
    const glm::vec3 right =
        glm::normalize(glm::cross(m_target, glm::vec3(0, 1, 0)));
    m_up = glm::normalize(glm::cross(right, m_target));
}

float Camera::GetFov() const { return FOV; }

void Camera::SetFOV(float fov) { FOV = fov; }

const glm::vec3& Camera::GetPos() const { return m_pos; }

const glm::vec3& Camera::GetTarget() const { return m_target; }

const glm::vec3& Camera::GetUp() const { return m_up; }

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAtRH(m_pos, m_pos + m_target, m_up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return glm::perspectiveRH_NO(glm::radians(FOV),
                                 static_cast<float>(m_windowWidth) /
                                     static_cast<float>(m_windowHeight),
                                 zNear, zFar);
}

glm::mat4 Camera::GetViewProjectionMatrix() const {
    return GetProjectionMatrix() * GetViewMatrix();
}

float Camera::GetZNear() const { return zNear; }

float Camera::GetZFar() const { return zFar; }

void Camera::SetZNear(float znear) { zNear = znear; }

void Camera::SetZFar(float zfar) { zFar = zfar; }

void Camera::OnViewportResize(int width, int height) {
    m_windowHeight = height;
    m_windowWidth = width;
    Init();
}

int Camera::GetWidth() const { return m_windowWidth; }

int Camera::GetHeight() const { return m_windowHeight; }
