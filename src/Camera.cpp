#include "Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

constexpr float kStepScale = 0.1F;
constexpr int kMargin = 100;

Camera::Camera(int WindowWidth, int WindowHeight, float fov, float z_near,
               float z_far) {
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    m_target = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    FOV = fov;
    zNear = z_near;
    zFar = z_far;

    Init();
}

Camera::Camera(int WindowWidth, int WindowHeight, float fov, float z_near,
               float z_far, const glm::vec3& Pos, const glm::vec3& Target,
               const glm::vec3& Up) {
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Pos;

    m_target = glm::normalize(Target);

    m_up = glm::normalize(Up);

    FOV = fov;
    zNear = z_near;
    zFar = z_far;

    Init();
}

void Camera::Init() {
    glm::vec3 HTarget(m_target.x, 0.0, m_target.z);
    HTarget = glm::normalize(HTarget);

    if (HTarget.z >= 0.0f) {
        if (HTarget.x >= 0.0f) {
            m_AngleH = 360.0f - glm::degrees(asin(HTarget.z));
        } else {
            m_AngleH = 180.0f + glm::degrees(asin(HTarget.z));
        }
    } else {
        if (HTarget.x >= 0.0f) {
            m_AngleH = glm::degrees(asin(-HTarget.z));
        } else {
            m_AngleH = 90.0f + glm::degrees(asin(-HTarget.z));
        }
    }

    m_AngleV = -glm::degrees(asin(m_target.y));

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
            m_pos += (m_target * kStepScale);
            Ret = true;
        } break;

        case GLFW_KEY_S: {
            m_pos -= (m_target * kStepScale);
            Ret = true;
        } break;

        case GLFW_KEY_A: {
            glm::vec3 Left = glm::cross(m_target, m_up);
            Left = glm::normalize(Left);
            Left *= kStepScale;
            m_pos += Left;
            Ret = true;
        } break;

        case GLFW_KEY_D: {
            glm::vec3 Right = glm::cross(m_up, m_target);
            Right = glm::normalize(Right);
            Right *= kStepScale;
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

    m_AngleH += static_cast<float>(DeltaX) / 20.0f;
    m_AngleV += static_cast<float>(DeltaY) / 20.0f;

    if (DeltaX == 0) {
        if (x <= kMargin) {
            // m_AngleH -= 1.0f;
            m_OnLeftEdge = true;
        } else if (x >= (m_windowWidth - kMargin)) {
            // m_AngleH += 1.0f;
            m_OnRightEdge = true;
        }
    } else {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (DeltaY == 0) {
        if (y <= kMargin) {
            m_OnUpperEdge = true;
        } else if (y >= (m_windowHeight - kMargin)) {
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
        m_AngleH -= 0.1f;
        ShouldUpdate = true;
    } else if (m_OnRightEdge) {
        m_AngleH += 0.1f;
        ShouldUpdate = true;
    }

    if (m_OnUpperEdge) {
        if (m_AngleV > -90.0f) {
            m_AngleV -= 0.1f;
            ShouldUpdate = true;
        }
    } else if (m_OnLowerEdge) {
        if (m_AngleV < 90.0f) {
            m_AngleV += 0.1f;
            ShouldUpdate = true;
        }
    }

    if (ShouldUpdate) {
        Update();
    }
}

void Camera::Update() {
    const glm::vec3 vertical_axis(0.0f, 1.0f, 0.0f);

    // Rotate the view vector by the horizontal angle around the vertical axis
    glm::vec3 View(1.0f, 0.0f, 0.0f);
    View = glm::rotate(View, m_AngleH, vertical_axis);
    View = glm::normalize(View);

    // Rotate the view vector by the vertical angle around the horizontal axis
    glm::vec3 horizontal_axis = glm::cross(vertical_axis, View);
    horizontal_axis = glm::normalize(horizontal_axis);
    View = glm::rotate(View, m_AngleV, horizontal_axis);

    m_target = glm::normalize(View);

    m_up = glm::cross(m_target, horizontal_axis);
    m_up = glm::normalize(m_up);
}

float Camera::GetFov() const { return FOV; }

void Camera::SetFOV(float fov) { FOV = fov; }

const glm::vec3& Camera::GetPos() const { return m_pos; }

const glm::vec3& Camera::GetTarget() const { return m_target; }

const glm::vec3& Camera::GetUp() const { return m_up; }

float Camera::GetZNear() const { return zNear; }

float Camera::GetZFar() const { return zFar; }

void Camera::SetZNear(float z_near) { zNear = z_near; }

void Camera::SetZFar(float z_far) { zFar = z_far; }

void Camera::OnViewportResize(int width, int height) {
    m_windowHeight = height;
    m_windowWidth = width;
    Init();
}

int Camera::GetWidth() const { return m_windowWidth; }

int Camera::GetHeight() const { return m_windowHeight; }
