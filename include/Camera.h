#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
    Camera(int WindowWidth, int WindowHeight, float fov, float znear,
           float zfar);

    Camera(int WindowWidth, int WindowHeight, float fov, float znear,
           float zfar, const glm::vec3& Pos, const glm::vec3& Target,
           const glm::vec3& Up);

    bool OnKeyboard(char Key);

    void OnMouse(int x, int y);

    void OnRender();

    const glm::vec3& GetPos() const;

    const glm::vec3& GetTarget() const;

    const glm::vec3& GetUp() const;

    float GetFov() const;

    void SetFOV(float fov);

    float GetZNear() const;

    float GetZFar() const;

    void SetZNear(float znear);

    void SetZFar(float zfar);

    void OnViewportResize(int width, int height);

    int GetWidth() const;

    int GetHeight() const;

private:
    float FOV;
    float zNear;
    float zFar;

    void Init();
    void Update();

    glm::vec3 m_pos;
    glm::vec3 m_target;
    glm::vec3 m_up;

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

#endif  // INCLUDE_CAMERA_H
