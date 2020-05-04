#ifndef ASSISTANT_H_INCLUDED
#define ASSISTANT_H_INCLUDED

#include "math_3d.h"

class Assistant {
public:
    glm::mat4 transformation;
    glm::mat4 temp;
    Assistant() {
        scale = glm::vec3(1.0f, 1.0f, 1.0f);
        worldPos = glm::vec3(0.0f, 0.0f, 0.0f);
        rotate = glm::vec3(0.0f, 0.0f, 0.0f);
        phi = 0;
    }
    void Scale(float ScaleX, float ScaleY, float ScaleZ) {
        scale.x = ScaleX;
        scale.y = ScaleY;
        scale.z = ScaleZ;
    }

    void WorldPos(float x, float y, float z) {
        worldPos.x = x;
        worldPos.y = y;
        worldPos.z = z;
    }
    void WorldPos(glm::vec3 v) {
        worldPos.x = v.x;
        worldPos.y = v.y;
        worldPos.z = v.z;
    }

    void Rotate(float RotateX, float RotateY, float RotateZ) {
        rotate.x = RotateX;
        rotate.y = RotateY;
        rotate.z = RotateZ;
    }

    void RotateOverVector(glm::vec3 v, float _phi) {
        rotateVector = v;
        phi = _phi;
    }

    void SetPerspectiveProj(float FOV, float Width, float Height, float zNear,
                            float zFar) {
        persProj.FOV = FOV;
        persProj.Width = Width;
        persProj.Height = Height;
        persProj.zNear = zNear;
        persProj.zFar = zFar;
    }

    void SetCamera(const glm::vec3& Pos, const glm::vec3& Target,
                   const glm::vec3& Up) {
        camera.Pos = Pos;
        camera.Target = Target;
        camera.Up = Up;
    }

    const glm::mat4& GetTSRVC();
    const glm::mat4& GetTSR();
    const glm::mat4& GetVC();
    const glm::mat4& GetRotate();
    const glm::mat4& GetCameraTrans();
    const glm::mat4& GetScaleTrans();
    // const glm::mat4* GetCamTransMat();

private:
    struct {
        float FOV;
        float Width;
        float Height;
        float zNear;
        float zFar;
    } persProj;
    struct {
        glm::vec3 Pos;
        glm::vec3 Target;
        glm::vec3 Up;
    } camera;
    glm::vec3 scale;
    glm::vec3 worldPos;
    glm::vec3 rotate;
    glm::vec3 rotateVector;
    float phi;
};
#endif  // ASSISTANT_H_INCLUDED
