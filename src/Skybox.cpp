#include "skybox.h"

#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/transform.hpp>

SkyBox::SkyBox(std::shared_ptr<Shader> shader) {
    shaderProgram = shader;
    shaderProgramID = shader->shaderProgramID;
}
SkyBox::~SkyBox() {}
bool SkyBox::Init(const std::string& filename) {
    Scale = 0;
    int* spindices = NULL;
    float* spvertices = NULL;
    float* spnormals = NULL;
    float* spuvs = NULL;
    float* sptangent = NULL;
    spfaces = 0;
    spverts = 0;
    try {
        /*FILE* fp;
                fp=fopen("models/sphere.ho3d","r+b");
                if(!fp) return false;
                fread(&spverts,sizeof(int),1,fp);
                spvertices=new float[spverts*3];
                spuvs=new float[spverts*2];
                spnormals=new float[spverts*3];
                for(int i=0;i<spverts;i++)
                        {
                        fread(&spvertices[3*i],sizeof(float),1,fp);
                        fread(&spvertices[3*i+1],sizeof(float),1,fp);
                        fread(&spvertices[3*i+2],sizeof(float),1,fp);
                        fread(&spuvs[2*i],sizeof(float),1,fp);
                        fread(&spuvs[2*i+1],sizeof(float),1,fp);
                        fread(&spnormals[3*i],sizeof(float),1,fp);
                        fread(&spnormals[3*i+1],sizeof(float),1,fp);
                        fread(&spnormals[3*i+2],sizeof(float),1,fp);
                        }
                fread(&spfaces,sizeof(int),1,fp);
                spindices=new int[spfaces*3];
                for(int i=0;i<spfaces;i++)
                        {
                                fread(&spindices[3*i],sizeof(int),1,fp);
                                fread(&spindices[3*i+1],sizeof(int),1,fp);
                                fread(&spindices[3*i+2],sizeof(int),1,fp);
                        }
                fclose(fp);*/
        FILE* fp;
        fp = fopen("models/sphere.ho3d", "r+b");
        if (!fp) return false;
        fread(&spverts, sizeof(int), 1, fp);
        spvertices = new float[spverts * 3];
        spuvs = new float[spverts * 2];
        spnormals = new float[spverts * 3];
        sptangent = new float;
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
        for (int i = 0; i < spverts * 3; i++) {
            fread(sptangent, sizeof(float), 1, fp);
        }
        fread(&spfaces, sizeof(int), 1, fp);
        spindices = new int[spfaces * 3];
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
    // создаём буффер, в котором будем хранить всё
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // создаём буффер
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * (spverts), nullptr,
                 GL_STATIC_DRAW);
    // загружаем вершины в буффер
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * spverts,
                    spvertices);

    // привязываем индексы к буфферу
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * spfaces, spindices,
                 GL_STATIC_DRAW);

    try {
        pCubemapTex = new CubemapTexture(filename);
    } catch (const std::bad_alloc&) {
        printf("\nError creating new Cubemap");
        return false;
    }
    pCubemapTex->Load();

    position_id = shaderProgram->GetAttribLocation("vertex_position");
    // находим позиции uniform-переменных
    model_id = shaderProgram->GetUniformLocation("gWVP");

    // делаем то же самое
    pCubemapTex->Bind(GL_TEXTURE2);
    textureID = shaderProgram->GetUniformLocation("gCubemapTexture");
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(textureID, 2);
    for (int i = 0; i < 3; i++) {
        position[i] = 0;
        model_rotation[i] = 0;
        scale[i] = 1;
    }

    // чистим память
    delete[] spindices;
    delete[] spvertices;
    delete[] spnormals;
    delete[] spuvs;
    delete sptangent;
    return true;
}

void SkyBox::Render(Camera* cam) {
    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glCullFace(GL_BACK);
    glDepthFunc(GL_LEQUAL);

    glm::mat4 model = glm::translate(cam->GetPos()) *
                      glm::orientate4(glm::vec3{180, 180, 0}) *
                      glm::scale(glm::vec3{3});
    glm::mat4 projection = glm::perspectiveFov(
        cam->GetFov(), static_cast<float>(cam->GetWidth()),
        static_cast<float>(cam->GetHeight()), cam->GetZNear(), cam->GetZFar());
    glm::mat4 view = glm::lookAt(cam->GetPos(), cam->GetTarget(), cam->GetUp());
    glm::mat4 mvp_matrix = projection * view * model;

    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    pCubemapTex->Bind(GL_TEXTURE2);
    glUniform1i(textureID, 2);
    // cout<<colTexID<<"\n"<<textureID<<"\n";

    glUniformMatrix4fv(model_id, 1, GL_TRUE, glm::value_ptr(mvp_matrix));

    glEnableVertexAttribArray(position_id);
    glDrawElements(GL_TRIANGLES, spfaces * 3, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(position_id);

    glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);
}
