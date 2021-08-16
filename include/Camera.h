#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
    Camera(int WindowWidth, int WindowHeight, float fov, float z_near,
           float z_far);

    Camera(int WindowWidth, int WindowHeight, float fov, float z_near,
           float z_far, const glm::vec3& Pos, const glm::vec3& Target,
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

    void SetZNear(float z_near);

    void SetZFar(float z_far);

    void OnViewportResize(int width, int height);

    int GetWidth() const;

    int GetHeight() const;

private:
    float fov_;
    float zNear_;
    float zFar_;

    void Init();
    void Update();

    glm::vec3 position_;
    glm::vec3 target_;
    glm::vec3 up_;

    int windowWidth_;
    int windowHeight_;

    float angleHorizontal_;
    float angleVertical_;

    bool onUpperEdge_{false};
    bool onLowerEdge_{false};
    bool onLeftEdge_{false};
    bool onRightEdge_{false};

    glm::vec2 mousePosition_;
};

#endif  // INCLUDE_CAMERA_H
