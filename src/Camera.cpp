#include "Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

constexpr float kStepScale = 0.1F;
constexpr int kMargin = 100;

Camera::Camera(int WindowWidth, int WindowHeight, float fov, float z_near,
               float z_far)
    : Camera(WindowWidth, WindowHeight, fov, z_near, z_far,
             glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
             glm::vec3(0.0f, 1.0f, 0.0f)) {}

Camera::Camera(int WindowWidth, int WindowHeight, float fov, float z_near,
               float z_far, const glm::vec3& Pos, const glm::vec3& Target,
               const glm::vec3& Up)
    : windowWidth_{WindowWidth},
      windowHeight_{WindowHeight},
      position_{Pos},
      target_{glm::normalize(Target)},
      up_{glm::normalize(Up)},
      fov_{fov},
      zNear_{z_near},
      zFar_{z_far} {
    Init();
}

void Camera::Init() {
    glm::vec3 HTarget(target_.x, 0.0, target_.z);
    HTarget = glm::normalize(HTarget);

    if (HTarget.z >= 0.0f) {
        if (HTarget.x >= 0.0f) {
            angleHorizontal_ = 360.0f - glm::degrees(asin(HTarget.z));
        } else {
            angleHorizontal_ = 180.0f + glm::degrees(asin(HTarget.z));
        }
    } else {
        if (HTarget.x >= 0.0f) {
            angleHorizontal_ = glm::degrees(asin(-HTarget.z));
        } else {
            angleHorizontal_ = 90.0f + glm::degrees(asin(-HTarget.z));
        }
    }

    angleVertical_ = -glm::degrees(asin(target_.y));

    onUpperEdge_ = false;
    onLowerEdge_ = false;
    onLeftEdge_ = false;
    onRightEdge_ = false;
    mousePosition_.x = windowWidth_ / 2;
    mousePosition_.y = windowHeight_ / 2;
    // glfwGetCursorPos(&mousePosition_.x, &mousePosition_.y);
    // glutWarpPointer(mousePosition_.x, mousePosition_.y);
}

bool Camera::OnKeyboard(char Key) {
    bool Ret = false;

    switch (Key) {
        case GLFW_KEY_W: {
            position_ += (target_ * kStepScale);
            Ret = true;
        } break;

        case GLFW_KEY_S: {
            position_ -= (target_ * kStepScale);
            Ret = true;
        } break;

        case GLFW_KEY_A: {
            glm::vec3 Left = glm::cross(target_, up_);
            Left = glm::normalize(Left);
            Left *= kStepScale;
            position_ += Left;
            Ret = true;
        } break;

        case GLFW_KEY_D: {
            glm::vec3 Right = glm::cross(up_, target_);
            Right = glm::normalize(Right);
            Right *= kStepScale;
            position_ += Right;
            Ret = true;
        } break;
    }

    return Ret;
}

void Camera::OnMouse(int x, int y) {
    const int DeltaX = x - mousePosition_.x;
    const int DeltaY = y - mousePosition_.y;
    // int DeltaX=x;
    // int DeltaY=y;

    mousePosition_.x = x;
    mousePosition_.y = y;

    angleHorizontal_ += static_cast<float>(DeltaX) / 20.0f;
    angleVertical_ += static_cast<float>(DeltaY) / 20.0f;

    if (DeltaX == 0) {
        if (x <= kMargin) {
            // angleHorizontal_ -= 1.0f;
            onLeftEdge_ = true;
        } else if (x >= (windowWidth_ - kMargin)) {
            // angleHorizontal_ += 1.0f;
            onRightEdge_ = true;
        }
    } else {
        onLeftEdge_ = false;
        onRightEdge_ = false;
    }

    if (DeltaY == 0) {
        if (y <= kMargin) {
            onUpperEdge_ = true;
        } else if (y >= (windowHeight_ - kMargin)) {
            onLowerEdge_ = true;
        }
    } else {
        onUpperEdge_ = false;
        onLowerEdge_ = false;
    }

    Update();
}

void Camera::OnRender() {
    bool ShouldUpdate = false;

    if (onLeftEdge_) {
        angleHorizontal_ -= 0.1f;
        ShouldUpdate = true;
    } else if (onRightEdge_) {
        angleHorizontal_ += 0.1f;
        ShouldUpdate = true;
    }

    if (onUpperEdge_) {
        if (angleVertical_ > -90.0f) {
            angleVertical_ -= 0.1f;
            ShouldUpdate = true;
        }
    } else if (onUpperEdge_) {
        if (angleVertical_ < 90.0f) {
            angleVertical_ += 0.1f;
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
    View = glm::rotate(View, angleHorizontal_, vertical_axis);
    View = glm::normalize(View);

    // Rotate the view vector by the vertical angle around the horizontal axis
    glm::vec3 horizontal_axis = glm::cross(vertical_axis, View);
    horizontal_axis = glm::normalize(horizontal_axis);
    View = glm::rotate(View, angleVertical_, horizontal_axis);

    target_ = glm::normalize(View);

    up_ = glm::cross(target_, horizontal_axis);
    up_ = glm::normalize(up_);
}

float Camera::GetFov() const { return fov_; }

void Camera::SetFOV(float fov) { fov_ = fov; }

const glm::vec3& Camera::GetPos() const { return position_; }

const glm::vec3& Camera::GetTarget() const { return target_; }

const glm::vec3& Camera::GetUp() const { return up_; }

float Camera::GetZNear() const { return zNear_; }

float Camera::GetZFar() const { return zFar_; }

void Camera::SetZNear(float z_near) { zNear_ = z_near; }

void Camera::SetZFar(float z_far) { zFar_ = z_far; }

void Camera::OnViewportResize(int width, int height) {
    windowHeight_ = height;
    windowWidth_ = width;
    Init();
}

int Camera::GetWidth() const { return windowWidth_; }

int Camera::GetHeight() const { return windowHeight_; }
