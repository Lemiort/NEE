#include "skybox.h"

#include <vector>

SkyBox::SkyBox(shared_ptr<Shader> shader) {
    shaderProgram = shader;
    shaderProgramID = shader->shaderProgramID;
}
SkyBox::~SkyBox() {}
bool SkyBox::Init(const string& Directory, const string& PosXFilename,
                  const string& NegXFilename, const string& PosYFilename,
                  const string& NegYFilename, const string& PosZFilename,
                  const string& NegZFilename) {
    Scale = 0;
    std::vector<int> spindices;
    std::vector<float> spvertices;
    std::vector<float> spnormals;
    std::vector<float> spuvs;
    std::vector<float> sptangent;
    spfaces = 0;
    spverts = 0;
    try {
        FILE* fp;
        fp = fopen("models/sphere.ho3d", "r+b");
        if (!fp) return false;
        fread(&spverts, sizeof(int), 1, fp);
        spvertices.resize(spverts * 3);
        spuvs.resize(spverts * 2);
        spnormals.resize(spverts * 3);
        sptangent.resize(1);  // placeholder size
        for (int i = 0; i < spverts; i++) {
            fread(&spvertices[3 * i], sizeof(float), 1, fp);
            fread(&spvertices[3 * i + 1], sizeof(float), 1, fp);
            fread(&spvertices[3 * i + 2], sizeof(float), 1, fp);
            fread(&spuvs[2 * i], sizeof(float), 1, fp);
            fread(&spuvs[2 * i + 1], sizeof(float), 1, fp);
            fread(&spnormals[3 * i], sizeof(float), 1, fp);
            fread(&spnormals[3 * i + 1], sizeof(float), 1, fp);
            fread(&spnormals[3 * i + 2], sizeof(float), 1, fp);
        }
        for (size_t i = 0; i < sptangent.size(); ++i) {
            fread(&sptangent[i], sizeof(float), 1, fp);
        }
        fread(&spfaces, sizeof(int), 1, fp);
        spindices.resize(spfaces * 3);
        for (int i = 0; i < spfaces; i++) {
            fread(&spindices[3 * i], sizeof(int), 1, fp);
            fread(&spindices[3 * i + 1], sizeof(int), 1, fp);
            fread(&spindices[3 * i + 2], sizeof(int), 1, fp);
        }
        fclose(fp);
    } catch (const std::bad_alloc&) {
        printf("\nError creating make_shared<Mesh> in Skybox");
        return false;
    }
    // create buffer to store everything
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // create buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * (spverts), nullptr,
                 GL_STATIC_DRAW);
    // load vertices into buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * spverts,
                    spvertices.data());

    // bind indices to buffer
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * spfaces,
                 spindices.data(), GL_STATIC_DRAW);

    try {
        pCubemapTex = std::make_unique<CubemapTexture>(
            Directory, PosXFilename, NegXFilename, PosYFilename, NegYFilename,
            PosZFilename, NegZFilename);
    } catch (const std::bad_alloc&) {
        printf("\nError creating new Cubemap");
        return false;
    }
    pCubemapTex->Load();

    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    // find uniform variable addresses
    gWorldID = shaderProgram->GetUniformLocation("gWVP");

    // do the same thing
    pCubemapTex->Bind(GL_TEXTURE2);
    textureID = shaderProgram->GetUniformLocation("gCubemapTexture");
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(textureID, 2);
    for (int i = 0; i < 3; i++) {
        position[i] = 0;
        rotation[i] = 0;
        scale[i] = 1;
    }

    // clean up memory
    // vectors automatically cleaned up
    return true;
}
void SkyBox::Render(const Camera& cam) {
    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glCullFace(GL_BACK);
    glDepthFunc(GL_LEQUAL);

    Assistant TM;  // TM - For object, 2- for object's normal, 3 - for camera position
    TM.Scale(3, 3, 3);
    TM.WorldPos(cam.GetPos().x, cam.GetPos().y, cam.GetPos().z);
    TM.Rotate(180, 180, 0);
    TM.SetCamera(cam.GetPos(), cam.GetTarget(), cam.GetUp());
    TM.SetPerspectiveProj(cam.GetFov(), cam.GetWidth(), cam.GetHeight(),
                          cam.GetZNear(), cam.GetZFar());

    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    pCubemapTex->Bind(GL_TEXTURE2);
    glUniform1i(textureID, 2);
    // cout<<colTexID<<"\n"<<textureID<<"\n";

    glUniformMatrix4fv(gWorldID, 1, GL_TRUE, (const GLfloat*)TM.GetTSRVC());

    glEnableVertexAttribArray(positionID);
    glDrawElements(GL_TRIANGLES, spfaces * 3, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(positionID);

    glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);
}
