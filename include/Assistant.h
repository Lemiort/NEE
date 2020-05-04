#ifndef ASSISTANT_H_INCLUDED
#define ASSISTANT_H_INCLUDED
#define _USE_MATH_DEFINES  // for C++
#include "math_3d.h"
class Assistant {
public:
    Matrix4f transformation;
    Matrix4f temp;
    Assistant() {
        scale = Vector3f(1.0f, 1.0f, 1.0f);
        worldPos = Vector3f(0.0f, 0.0f, 0.0f);
        rotate = Vector3f(0.0f, 0.0f, 0.0f);
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
    void WorldPos(Vector3f v) {
        worldPos.x = v.x;
        worldPos.y = v.y;
        worldPos.z = v.z;
    }

    void Rotate(float RotateX, float RotateY, float RotateZ) {
        rotate.x = RotateX;
        rotate.y = RotateY;
        rotate.z = RotateZ;
    }

    void RotateOverVector(Vector3f v, float _phi) {
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

    void SetCamera(const Vector3f& Pos, const Vector3f& Target,
                   const Vector3f& Up) {
        camera.Pos = Pos;
        camera.Target = Target;
        camera.Up = Up;
    }

    const Matrix4f* GetTSRVC();
    const Matrix4f* GetTSR();
    const Matrix4f* GetVC();
    const Matrix4f* GetRotate();
    const Matrix4f* GetCameraTrans();
    const Matrix4f* GetScaleTrans();
    // const Matrix4f* GetCamTransMat();

private:
    struct {
        float FOV;
        float Width;
        float Height;
        float zNear;
        float zFar;
    } persProj;
    struct {
        Vector3f Pos;
        Vector3f Target;
        Vector3f Up;
    } camera;
    Vector3f scale;
    Vector3f worldPos;
    Vector3f rotate;
    Vector3f rotateVector;
    float phi;
};
#endif  // ASSISTANT_H_INCLUDED
