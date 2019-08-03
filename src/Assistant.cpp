#include "Assistant.h"
#include <fstream>

void Assistant::InitScaleTransform(Matrix4f& m) const {
    m.m[0][0] = scale.x;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;
    m.m[1][0] = 0.0f;
    m.m[1][1] = scale.y;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = scale.z;
    m.m[2][3] = 0.0f;
    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;
}

void Assistant::InitRotateTransform(Matrix4f& m) const {
    Matrix4f rx, ry, rz;

    const float x = ToRadian(rotate.x);
    const float y = ToRadian(rotate.y);
    const float z = ToRadian(rotate.z);

    rx.m[0][0] = 1.0f;
    rx.m[0][1] = 0.0f;
    rx.m[0][2] = 0.0f;
    rx.m[0][3] = 0.0f;
    rx.m[1][0] = 0.0f;
    rx.m[1][1] = cosf(x);
    rx.m[1][2] = -sinf(x);
    rx.m[1][3] = 0.0f;
    rx.m[2][0] = 0.0f;
    rx.m[2][1] = sinf(x);
    rx.m[2][2] = cosf(x);
    rx.m[2][3] = 0.0f;
    rx.m[3][0] = 0.0f;
    rx.m[3][1] = 0.0f;
    rx.m[3][2] = 0.0f;
    rx.m[3][3] = 1.0f;

    ry.m[0][0] = cosf(y);
    ry.m[0][1] = 0.0f;
    ry.m[0][2] = -sinf(y);
    ry.m[0][3] = 0.0f;
    ry.m[1][0] = 0.0f;
    ry.m[1][1] = 1.0f;
    ry.m[1][2] = 0.0f;
    ry.m[1][3] = 0.0f;
    ry.m[2][0] = sinf(y);
    ry.m[2][1] = 0.0f;
    ry.m[2][2] = cosf(y);
    ry.m[2][3] = 0.0f;
    ry.m[3][0] = 0.0f;
    ry.m[3][1] = 0.0f;
    ry.m[3][2] = 0.0f;
    ry.m[3][3] = 1.0f;

    rz.m[0][0] = cosf(z);
    rz.m[0][1] = -sinf(z);
    rz.m[0][2] = 0.0f;
    rz.m[0][3] = 0.0f;
    rz.m[1][0] = sinf(z);
    rz.m[1][1] = cosf(z);
    rz.m[1][2] = 0.0f;
    rz.m[1][3] = 0.0f;
    rz.m[2][0] = 0.0f;
    rz.m[2][1] = 0.0f;
    rz.m[2][2] = 1.0f;
    rz.m[2][3] = 0.0f;
    rz.m[3][0] = 0.0f;
    rz.m[3][1] = 0.0f;
    rz.m[3][2] = 0.0f;
    rz.m[3][3] = 1.0f;

    m = rz * ry * rx;

    /*Matrix4f rv;
    const float phi = ToRadian(this->phi);
    const Vector3f &v = rotateVector;

    rv.m[0][0] = cos(phi) + (1 - cos(phi))*v.x*v.x	;
        rv.m[1][0] = (1-cos(phi))*v.y*v.x + sin(phi)*v.z;
        rv.m[2][0] = (1-cos(phi))*v.z*v.x - sin(phi)*v.y;
        rv.m[3][0] = 0.0f
    ;

        rv.m[0][1] = (1-cos(phi))*v.x*v.y - sin(phi)*v.z	;
        rv.m[1][1] = cos(phi) + (1-cos(phi))*v.y*v.y	;
        rv.m[2][1] = (1-cos(phi))*v.z*v.y + sin(phi)*v.x;
        rv.m[3][1] = 0.0f
    ;

        rv.m[0][2] = (1-cos(phi))*v.x*v.z + sin(phi)*v.y;
        rv.m[1][2] = (1-cos(phi))*v.y*v.z - sin(phi)*v.x;
        rv.m[2][2] = cos(phi) + (1-cos(phi))*v.z*v.z	;
        rv.m[3][2] = 0.0f
    ;

        rv.m[0][3] = 0.0f
    ; rv.m[1][3] = 0.0f ; rv.m[2][3] = 0.0f
    ; rv.m[3][3] = 1.0f ;

        m = rv * rz * ry * rx;*/
}

void Assistant::InitTranslationTransform(Matrix4f& m) const {
    m.m[0][0] = 1.0f;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = worldPos.x;
    m.m[1][0] = 0.0f;
    m.m[1][1] = 1.0f;
    m.m[1][2] = 0.0f;
    m.m[1][3] = worldPos.y;
    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = 1.0f;
    m.m[2][3] = worldPos.z;
    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;
}

void Assistant::InitPerspectiveProj(Matrix4f& m) const {
    const float ar = persProj.Width / persProj.Height;
    const float zNear = persProj.zNear;
    const float zFar = persProj.zFar;
    const float zRange = zNear - zFar;
    const float tanHalfFOV = tanf(ToRadian(persProj.FOV / 2.0f));

    m.m[0][0] = 1.0f / (tanHalfFOV * ar);
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0;
    m.m[1][0] = 0.0f;
    m.m[1][1] = 1.0f / tanHalfFOV;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0;
    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = (-zNear - zFar) / zRange;
    m.m[2][3] = 2.0f * zFar * zNear / zRange;
    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = 1.0f;
    m.m[3][3] = 0.0;
}

const Matrix4f* Assistant::GetTSRVC() {
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans, PersProjTrans,
        CameraTranslationTrans, CameraRotateTrans;

    ScaleTrans.InitScaleTransform(scale.x, scale.y, scale.z);
    RotateTrans.InitRotateTransform(rotate.x, rotate.y, rotate.z);
    TranslationTrans.InitTranslationTransform(worldPos.x, worldPos.y,
                                              worldPos.z);
    CameraTranslationTrans.InitTranslationTransform(
        -camera.Pos.x, -camera.Pos.y, -camera.Pos.z);
    CameraRotateTrans.InitCameraTransform(camera.Target, camera.Up);
    PersProjTrans.InitPersProjTransform(persProj.FOV, persProj.Width,
                                        persProj.Height, persProj.zNear,
                                        persProj.zFar);

    transformation = PersProjTrans * CameraRotateTrans *
                     CameraTranslationTrans * TranslationTrans * RotateTrans *
                     ScaleTrans;
    return &transformation;
}

const Matrix4f* Assistant::GetTSR() {
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans;

    ScaleTrans.InitScaleTransform(scale.x, scale.y, scale.z);
    RotateTrans.InitRotateTransform(rotate.x, rotate.y, rotate.z);
    TranslationTrans.InitTranslationTransform(worldPos.x, worldPos.y,
                                              worldPos.z);
    transformation = TranslationTrans * RotateTrans * ScaleTrans;
    return &transformation;
}

const Matrix4f* Assistant::GetVC() {
    Matrix4f PersProjTrans, CameraTranslationTrans, CameraRotateTrans;

    CameraTranslationTrans.InitTranslationTransform(
        -camera.Pos.x, -camera.Pos.y, -camera.Pos.z);
    CameraRotateTrans.InitCameraTransform(camera.Target, camera.Up);
    PersProjTrans.InitPersProjTransform(persProj.FOV, persProj.Width,
                                        persProj.Height, persProj.zNear,
                                        persProj.zFar);

    transformation = PersProjTrans * CameraRotateTrans * CameraTranslationTrans;
    return &transformation;
}

const Matrix4f* Assistant::GetRotate() {
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans;
    ScaleTrans.InitScaleTransform(scale.x, scale.y, scale.z);
    RotateTrans.InitRotateTransform(rotate.x, rotate.y, rotate.z);
    TranslationTrans.InitTranslationTransform(worldPos.x, worldPos.y,
                                              worldPos.z);
    /* std::fstream fout;
    fout.open("out.txt",std::ios::out);
            for(int i=0;i<4;i++)
                    {
                    fout<<'\n';
                            for(int j=0;j<4;j++)
                            {
                                    fout<<RotateTrans.m[i][j]<<" ";
                            }
                    }
            fout<<'\n\n';*/
    // fout.close();
    transformation = TranslationTrans * RotateTrans * ScaleTrans;
    // return &RotateTrans;
    return &transformation;
}
const Matrix4f* Assistant::GetScaleTrans() {
    Matrix4f ScaleTrans;
    ScaleTrans.InitScaleTransform(scale.x, scale.y, scale.z);
    transformation = ScaleTrans;
    return &transformation;
}
const Matrix4f* Assistant::GetCameraTrans() {
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans, PersProjTrans,
        CameraTranslationTrans, CameraRotateTrans;
    CameraTranslationTrans.InitTranslationTransform(
        -camera.Pos.x, -camera.Pos.y, -camera.Pos.z);
    // PersProjTrans.InitPersProjTransform(persProj.FOV, persProj.Width,
    // persProj.Height, persProj.zNear, persProj.zFar);
    transformation = CameraTranslationTrans;
    return &transformation;
}
