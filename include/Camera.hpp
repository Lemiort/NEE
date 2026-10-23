#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int2.hpp>

class Camera {
public:
    Camera(int WindowWidth, int WindowHeight, float fov, float znear,
           float zfar);

    Camera(int WindowWidth, int WindowHeight, float fov, float znear,
           float zfar, const glm::vec3& Pos, const glm::vec3& Target,
           const glm::vec3& Up);

    bool OnKeyboard(int key);

    void OnMouse(int x, int y);

    void OnRender();

    [[nodiscard]] const glm::vec3& GetPos() const;

    [[nodiscard]] const glm::vec3& GetTarget() const;

    [[nodiscard]] const glm::vec3& GetUp() const;

    [[nodiscard]] glm::mat4 GetViewMatrix() const;
    [[nodiscard]] glm::mat4 GetProjectionMatrix() const;
    [[nodiscard]] glm::mat4 GetViewProjectionMatrix() const;

    [[nodiscard]] float GetFov() const;

    void SetFOV(float fov);

    [[nodiscard]] float GetZNear() const;

    [[nodiscard]] float GetZFar() const;

    void SetZNear(float znear);

    void SetZFar(float zfar);

    void OnViewportResize(int width, int height);

    [[nodiscard]] int GetWidth() const;

    [[nodiscard]] int GetHeight() const;

private:
    float FOV;
    float zNear;
    float zFar;

    void ResetInteractionState();
    void Update();

    glm::vec3 m_pos;
    glm::vec3 m_target;
    glm::vec3 m_up;

    int m_windowWidth;
    int m_windowHeight;

    float m_AngleH{};
    float m_AngleV{};

    bool m_OnUpperEdge{};
    bool m_OnLowerEdge{};
    bool m_OnLeftEdge{};
    bool m_OnRightEdge{};

    glm::ivec2 m_mousePos;
};

#endif  // CAMERA_H_INCLUDED
