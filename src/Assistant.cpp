#include "Assistant.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

const glm::mat4& Assistant::GetTSRVC() {
    glm::mat4 ScaleTrans, RotateTrans, TranslationTrans, PersProjTrans,
        CameraTrans;

    ScaleTrans = glm::scale(scale);
    RotateTrans = glm::orientate4(rotate);
    TranslationTrans = glm::translate(worldPos);
    CameraTrans = glm::lookAt(camera.Pos, camera.Target, camera.Up);
    PersProjTrans =
        glm::perspectiveFov(persProj.FOV, persProj.Width, persProj.Height,
                            persProj.zNear, persProj.zFar);

    transformation = PersProjTrans * CameraTrans * TranslationTrans *
                     RotateTrans * ScaleTrans;
    // transformation = CameraTrans * PersProjTrans * RotateTrans * ScaleTrans *
    //                  TranslationTrans;
    return transformation;
}

const glm::mat4& Assistant::GetTSR() {
    glm::mat4 ScaleTrans, RotateTrans, TranslationTrans;

    ScaleTrans = glm::scale(scale);
    RotateTrans = glm::orientate4(rotate);
    TranslationTrans = glm::translate(worldPos);
    transformation = TranslationTrans * RotateTrans * ScaleTrans;
    // transformation = RotateTrans * ScaleTrans * TranslationTrans;
    return transformation;
}

const glm::mat4& Assistant::GetVC() {
    glm::mat4 PersProjTrans, CameraTrans;
    CameraTrans = glm::lookAt(camera.Pos, camera.Target, camera.Up);
    PersProjTrans =
        glm::perspectiveFov(persProj.FOV, persProj.Width, persProj.Height,
                            persProj.zNear, persProj.zFar);

    transformation = PersProjTrans * CameraTrans;
    // transformation = CameraTrans * PersProjTrans;
    return transformation;
}

const glm::mat4& Assistant::GetRotate() {
    glm::mat4 ScaleTrans, RotateTrans, TranslationTrans;
    ScaleTrans = glm::scale(scale);
    RotateTrans = glm::orientate4(rotate);
    TranslationTrans = glm::translate(worldPos);
    transformation = TranslationTrans * RotateTrans * ScaleTrans;
    return transformation;
}
