#include "skybox.h"

#include <spdlog/spdlog.h>

#include <glm/gtc/type_ptr.hpp>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/transform.hpp>

SkyBox::SkyBox(std::shared_ptr<Shader> shader) {
    shaderProgram = shader;
    shaderProgramID = shader->shaderProgramID;
}

bool SkyBox::Init(const std::string& filename) {
    Scale = 0;
    std::vector<int> indices;
    std::vector<float> vertexes_buffer;
    std::vector<float> normals_buffer;
    std::vector<float> uvs_buffer;
    std::vector<float> tangents_buffer;
    faces_count = 0;
    vertices_count = 0;
    try {
        FILE* fp;
        fp = fopen("models/sphere.ho3d", "r+b");
        if (!fp) return false;
        fread(&vertices_count, sizeof(int), 1, fp);
        vertexes_buffer.resize(vertices_count * 3);
        uvs_buffer.resize(vertices_count * 2);
        normals_buffer.resize(vertices_count * 3);
        tangents_buffer.resize(vertices_count * 3);
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
            fread(tangents_buffer.data(), sizeof(float), 1, fp);
        }
        fread(&faces_count, sizeof(int), 1, fp);
        indices.resize(faces_count * 3);
        for (int i = 0; i < faces_count; i++) {
            fread(&indices[3 * i], sizeof(int), 1, fp);
            fread(&indices[3 * i + 1], sizeof(int), 1, fp);
            fread(&indices[3 * i + 2], sizeof(int), 1, fp);
        }
        fclose(fp);
    } catch (const std::bad_alloc&) {
        spdlog::error("Error mem allocation");
        return false;
    }
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexes_buffer.size(),
                 nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertexes_buffer.size(),
                    vertexes_buffer.data());

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    cubemapTexture = CubemapTexture(filename);
    cubemapTexture.Load();

    position_id = shaderProgram->GetAttribLocation("vertex_position");
    model_id = shaderProgram->GetUniformLocation("gWVP");

    cubemapTexture.Bind(GL_TEXTURE2);
    textureID = shaderProgram->GetUniformLocation("gCubemapTexture");
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(textureID, 2);
    for (int i = 0; i < 3; i++) {
        position[i] = 0;
        model_rotation[i] = 0;
        scale[i] = 1;
    }
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

    cubemapTexture.Bind(GL_TEXTURE2);
    glUniform1i(textureID, 2);

    glUniformMatrix4fv(model_id, 1, GL_TRUE, glm::value_ptr(mvp_matrix));

    glEnableVertexAttribArray(position_id);
    glDrawElements(GL_TRIANGLES, faces_count * 3, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(position_id);

    glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);
}
