#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "math_3d.h"
#include <glm/glm.hpp>

class Camera {
public:
    Camera(int WindowWidth, int WindowHeight, float fov, float znear,
           float zfar);

    Camera(int WindowWidth, int WindowHeight, float fov, float znear,
           float zfar, const Vector3f& Pos, const Vector3f& Target,
           const Vector3f& Up);

    bool OnKeyboard(char Key);

    void OnMouse(int x, int y);

    void OnRender();

    const Vector3f& GetPos() const;

    const Vector3f& GetTarget() const;

    const Vector3f& GetUp() const;

    float GetFov();

    void SetFOV(float fov);

    float GetZNear();

    float GetZFar();

    void SetZNear(float znear);

    void SetZFar(float zfar);

    void OnViewportResize(int width, int height);

    int GetWidth();

    int GetHeight();

private:
    float FOV;
    float zNear;
    float zFar;

    void Init();
    void Update();

    Vector3f m_pos;
    Vector3f m_target;
    Vector3f m_up;

    int m_windowWidth;
    int m_windowHeight;

    float m_AngleH;
    float m_AngleV;

    bool m_OnUpperEdge;
    bool m_OnLowerEdge;
    bool m_OnLeftEdge;
    bool m_OnRightEdge;

    glm::vec2 m_mousePos;
};

#endif  // CAMERA_H_INCLUDED
