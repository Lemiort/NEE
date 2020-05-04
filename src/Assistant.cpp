#include "Assistant.h"

const Matrix4f* Assistant::GetTSRVC() {
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans, PersProjTrans,
        CameraTrans;

    ScaleTrans = glm::scale(scale);
    RotateTrans = glm::orientate4(rotate);
    TranslationTrans = glm::translate(worldPos);
    CameraTrans = glm::lookAt(camera.Pos, camera.Target, camera.Up);
    PersProjTrans = glm::perspective(persProj.FOV, persProj.Width,
                                        persProj.Height, persProj.zNear,
                                        persProj.zFar);

    transformation = PersProjTrans * CameraTrans * TranslationTrans * RotateTrans *
                     ScaleTrans;
    return &transformation;
}

const Matrix4f* Assistant::GetTSR() {
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans;

    ScaleTrans = glm::scale(scale);
    RotateTrans = glm::orientate4(rotate);
    TranslationTrans = glm::translate(worldPos);
    transformation = TranslationTrans * RotateTrans * ScaleTrans;
    return &transformation;
}

const Matrix4f* Assistant::GetVC() {
    Matrix4f PersProjTrans, CameraTrans;
    CameraTrans = glm::lookAt(camera.Pos, camera.Target, camera.Up);
    PersProjTrans = glm::perspective(persProj.FOV, persProj.Width,
                                        persProj.Height, persProj.zNear,
                                        persProj.zFar);

    transformation = PersProjTrans * CameraTrans;
    return &transformation;
}

const Matrix4f* Assistant::GetRotate() {
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans;
    ScaleTrans= glm::scale(scale);
    RotateTrans = glm::orientate4(rotate);
    TranslationTrans = glm::translate(worldPos);
    transformation = TranslationTrans * RotateTrans * ScaleTrans;
    return &transformation;
}
const Matrix4f* Assistant::GetScaleTrans() {
    Matrix4f ScaleTrans;
    ScaleTrans = glm::scale(scale);
    transformation = ScaleTrans;
    return &transformation;
}
const Matrix4f* Assistant::GetCameraTrans() {
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans, PersProjTrans,
        CameraTranslationTrans, CameraRotateTrans;
    CameraTranslationTrans = glm::translate(-camera.Pos);
    transformation = CameraTranslationTrans;
    return &transformation;
}
