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
    float* vertexes_buffer = NULL;
    float* normals_buffer = NULL;
    float* uvs_buffer = NULL;
    float* tangents_buffer = NULL;
    faces_count = 0;
    vertices_count = 0;
    try {
        /*FILE* fp;
                fp=fopen("models/sphere.ho3d","r+b");
                if(!fp) return false;
                fread(&vertices_count,sizeof(int),1,fp);
                vertexes_buffer=new float[vertices_count*3];
                uvs_buffer=new float[vertices_count*2];
                normals_buffer=new float[vertices_count*3];
                for(int i=0;i<vertices_count;i++)
                        {
                        fread(&vertexes_buffer[3*i],sizeof(float),1,fp);
                        fread(&vertexes_buffer[3*i+1],sizeof(float),1,fp);
                        fread(&vertexes_buffer[3*i+2],sizeof(float),1,fp);
                        fread(&uvs_buffer[2*i],sizeof(float),1,fp);
                        fread(&uvs_buffer[2*i+1],sizeof(float),1,fp);
                        fread(&normals_buffer[3*i],sizeof(float),1,fp);
                        fread(&normals_buffer[3*i+1],sizeof(float),1,fp);
                        fread(&normals_buffer[3*i+2],sizeof(float),1,fp);
                        }
                fread(&faces_count,sizeof(int),1,fp);
                spindices=new int[faces_count*3];
                for(int i=0;i<faces_count;i++)
                        {
                                fread(&spindices[3*i],sizeof(int),1,fp);
                                fread(&spindices[3*i+1],sizeof(int),1,fp);
                                fread(&spindices[3*i+2],sizeof(int),1,fp);
                        }
                fclose(fp);*/
        FILE* fp;
        fp = fopen("models/sphere.ho3d", "r+b");
        if (!fp) return false;
        fread(&vertices_count, sizeof(int), 1, fp);
        vertexes_buffer = new float[vertices_count * 3];
        uvs_buffer = new float[vertices_count * 2];
        normals_buffer = new float[vertices_count * 3];
        tangents_buffer = new float;
        for (int i = 0; i < vertices_count; i++) {
            fread(&vertexes_buffer[3 * i], sizeof(float), 1, fp);
            fread(&vertexes_buffer[3 * i + 1], sizeof(float), 1, fp);
            fread(&vertexes_buffer[3 * i + 2], sizeof(float), 1, fp);
            fread(&uvs_buffer[2 * i], sizeof(float), 1, fp);
            fread(&uvs_buffer[2 * i + 1], sizeof(float), 1, fp);
            fread(&normals_buffer[3 * i], sizeof(float), 1, fp);
            fread(&normals_buffer[3 * i + 1], sizeof(float), 1, fp);
            fread(&normals_buffer[3 * i + 2], sizeof(float), 1, fp);
        }
        for (int i = 0; i < vertices_count * 3; i++) {
            fread(tangents_buffer, sizeof(float), 1, fp);
        }
        fread(&faces_count, sizeof(int), 1, fp);
        spindices = new int[faces_count * 3];
        for (int i = 0; i < faces_count; i++) {
            fread(&spindices[3 * i], sizeof(int), 1, fp);
            fread(&spindices[3 * i + 1], sizeof(int), 1, fp);
            fread(&spindices[3 * i + 2], sizeof(int), 1, fp);
        }
        fclose(fp);
    } catch (const std::bad_alloc&) {
        printf("\nError creating make_shared<Mesh> in Skybox");
        return false;
    }
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * (vertices_count), nullptr,
                 GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * vertices_count,
                    vertexes_buffer);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * faces_count,
                 spindices, GL_STATIC_DRAW);

    try {
        pCubemapTex = new CubemapTexture(filename);
    } catch (const std::bad_alloc&) {
        printf("\nError creating new Cubemap");
        return false;
    }
    pCubemapTex->Load();

    position_id = shaderProgram->GetAttribLocation("vertex_position");
    model_id = shaderProgram->GetUniformLocation("gWVP");

    pCubemapTex->Bind(GL_TEXTURE2);
    textureID = shaderProgram->GetUniformLocation("gCubemapTexture");
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(textureID, 2);
    for (int i = 0; i < 3; i++) {
        position[i] = 0;
        model_rotation[i] = 0;
        scale[i] = 1;
    }

    delete[] spindices;
    delete[] vertexes_buffer;
    delete[] normals_buffer;
    delete[] uvs_buffer;
    delete tangents_buffer;
    return true;
}

void SkyBox::Render(const Camera& cam) {
    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glCullFace(GL_BACK);
    glDepthFunc(GL_LEQUAL);

    glm::mat4 model = glm::translate(cam.GetPos()) *
                      glm::orientate4(glm::vec3{180, 180, 0}) *
                      glm::scale(glm::vec3{3});
    glm::mat4 projection = glm::perspectiveFov(
        cam.GetFov(), static_cast<float>(cam.GetWidth()),
        static_cast<float>(cam.GetHeight()), cam.GetZNear(), cam.GetZFar());
    glm::mat4 view = glm::lookAt(cam.GetPos(), cam.GetTarget(), cam.GetUp());
    glm::mat4 vp_matrix = glm::transpose(projection * view);
    glm::mat4 mvp_matrix = vp_matrix * model;

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
    glDrawElements(GL_TRIANGLES, faces_count * 3, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(position_id);

    glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);
}
