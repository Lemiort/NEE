#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>  // vec2, vec3, mat4, radians

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

    glm::ivec2 m_mousePos;
};
#endif  // CAMERA_H_INCLUDED
