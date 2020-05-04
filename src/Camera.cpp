#include "Camera.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

const static float STEP_SCALE = 0.1f;
const static int MARGIN = 100;

Camera::Camera(int WindowWidth, int WindowHeight, float fov, float znear,
               float zfar) {
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Vector3f(0.0f, 0.0f, 0.0f);
    m_target = Vector3f(0.0f, 0.0f, -1.0f);
    m_up = Vector3f(0.0f, 1.0f, 0.0f);
    FOV = fov;
    zNear = znear;
    zFar = zfar;

    Init();
}

Camera::Camera(int WindowWidth, int WindowHeight, float fov, float znear,
               float zfar, const Vector3f& Pos, const Vector3f& Target,
               const Vector3f& Up) {
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Pos;

    m_target = glm::normalize(Target);

    m_up = glm::normalize(Up);

    FOV = fov;
    zNear = znear;
    zFar = zfar;

    Init();
}

void Camera::Init() {
    Vector3f HTarget(m_target.x, 0.0, m_target.z);
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
            m_pos += (m_target * STEP_SCALE);
            Ret = true;
        } break;

        case GLFW_KEY_S: {
            m_pos -= (m_target * STEP_SCALE);
            Ret = true;
        } break;

        case GLFW_KEY_A: {
            Vector3f Left = glm::cross(m_target,m_up);
            Left = glm::normalize(Left);
            Left *= STEP_SCALE;
            m_pos += Left;
            Ret = true;
        } break;

        case GLFW_KEY_D: {
            Vector3f Right = glm::cross(m_up,m_target);
            Right = glm::normalize(Right);
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

    m_AngleH += (float)DeltaX / 20.0f;
    m_AngleV += (float)DeltaY / 20.0f;

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
    const Vector3f Vaxis(0.0f, 1.0f, 0.0f);

    // Rotate the view vector by the horizontal angle around the vertical axis
    Vector3f View(1.0f, 0.0f, 0.0f);
    View = glm::rotate(View,m_AngleH, Vaxis);
    View = glm::normalize(View);

    // Rotate the view vector by the vertical angle around the horizontal axis
    Vector3f Haxis = glm::cross(Vaxis,View);
    Haxis = glm::normalize(Haxis);
    View = glm::rotate(View,m_AngleV, Haxis);

    m_target = glm::normalize(View);

    m_up = glm::cross(m_target,Haxis);
    m_up = glm::normalize(m_up);
}

float Camera::GetFov() { return FOV; }

void Camera::SetFOV(float fov) { FOV = fov; }

const Vector3f& Camera::GetPos() const { return m_pos; }

const Vector3f& Camera::GetTarget() const { return m_target; }

const Vector3f& Camera::GetUp() const { return m_up; }

float Camera::GetZNear() { return zNear; }

float Camera::GetZFar() { return zFar; }

void Camera::SetZNear(float znear) { zNear = znear; }

void Camera::SetZFar(float zfar) { zFar = zfar; }

void Camera::OnViewportResize(int width, int height) {
    m_windowHeight = height;
    m_windowWidth = width;
    Init();
}

int Camera::GetWidth() { return m_windowWidth; }

int Camera::GetHeight() { return m_windowHeight; }
