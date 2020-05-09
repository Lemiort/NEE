#include "Mesh.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/transform.hpp>

Mesh::Mesh() {
    // mat=NULL;
    shadowMap = 0;
    rotation_angle = 0;
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}

void Mesh::SetMaterial(std::shared_ptr<Material> _mat) {
    mat = _mat;
    this->SetShader(mat->GetShader());

    // mat->Use();
    position_id = shaderProgram->GetAttribLocation("vertex_position");
    normal_id = shaderProgram->GetAttribLocation("vertex_normal");
    uv_id = shaderProgram->GetAttribLocation("vertex_uv");
    tangent_id = shaderProgram->GetAttribLocation("vertex_tangent");
    model_id = shaderProgram->GetUniformLocation("model");
    rotation_id = shaderProgram->GetUniformLocation("model_rotation");
}

bool Mesh::Init(std::shared_ptr<Material> _mat, const char* model) {
    mat = _mat;
    shaderProgram = mat->GetShader();
    // mat->Use();

    if (shaderProgram == NULL) return false;

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
        std::ofstream fout("ImportObj.log");
        // fopen_s(&fp,model,"r+b");
        fp = fopen(model, "r+b");
        if (!fp) return false;
        fread(&vertices_count, sizeof(int), 1, fp);
        fout << vertices_count << std::endl;
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
            fread(&tangents_buffer[i], sizeof(float), 1, fp);
        }
        for (int i = 0; i < vertices_count; i++) {
            fout << i << " ";
            fout << vertexes_buffer[3 * i] << ",";
            fout << vertexes_buffer[3 * i + 1] << ",";
            fout << vertexes_buffer[3 * i + 2] << ",";
            fout << uvs_buffer[2 * i + 0] << ",";
            fout << uvs_buffer[2 * i + 1] << ",";
            fout << normals_buffer[3 * i + 0] << ",";
            fout << normals_buffer[3 * i + 1] << ",";
            fout << normals_buffer[3 * i + 2] << ",\n";
        }
        // vertices[0]=f;
        fread(&faces_count, sizeof(int), 1, fp);
        fout << "\n" << faces_count << std::endl;
        indices.resize(faces_count * 3);
        for (int i = 0; i < faces_count; i++) {
            fread(&indices[3 * i], sizeof(int), 1, fp);
            fread(&indices[3 * i + 1], sizeof(int), 1, fp);
            fread(&indices[3 * i + 2], sizeof(int), 1, fp);
        }
        fclose(fp);
        for (int i = 0; i < faces_count; i++) {
            fout << indices[3 * i] << ",";
            fout << indices[3 * i + 1] << "," << indices[3 * i + 2]
                 << std::endl;
        }
        fout.close();
    } catch (const std::bad_alloc&) {
        printf("\nError creating make_shared<Mesh>");
        return false;
    }

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 (vertexes_buffer.size() + normals_buffer.size() +
                  uvs_buffer.size() + tangents_buffer.size()) *
                     sizeof(float),
                 nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexes_buffer.size() * sizeof(float),
                    vertexes_buffer.data());
    glBufferSubData(GL_ARRAY_BUFFER, vertexes_buffer.size() * sizeof(float),
                    normals_buffer.size() * sizeof(float),
                    normals_buffer.data());
    glBufferSubData(
        GL_ARRAY_BUFFER,
        sizeof(float) * (vertexes_buffer.size() + normals_buffer.size()),
        uvs_buffer.size() * sizeof(float), uvs_buffer.data());
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(float) * (vertexes_buffer.size() +
                                     normals_buffer.size() + uvs_buffer.size()),
                    tangents_buffer.size() * sizeof(float),
                    tangents_buffer.data());

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    position_id = shaderProgram->GetAttribLocation("vertex_position");
    normal_id = shaderProgram->GetAttribLocation("vertex_normal");
    uv_id = shaderProgram->GetAttribLocation("vertex_uv");
    tangent_id = shaderProgram->GetAttribLocation("vertex_tangent");
    model_id = shaderProgram->GetUniformLocation("model");
    rotation_id = shaderProgram->GetUniformLocation("model_rotation");

    for (int i = 0; i < 3; i++) {
        position[i] = 0;
        model_rotation[i] = 0;
        scale[i] = 1;
    }

    return true;
}
void Mesh::Render(const Camera& cam) {
    glm::mat4 model = glm::translate(position) *
                      glm::orientate4(model_rotation) * glm::scale(scale);
    model = glm::transpose(model);
    auto rotate_matrix =
        glm::diagonal4x4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));  // TODO fix

    mat->Use();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(
        normal_id, 3, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<const void*>(sizeof(float) * 3 * vertices_count));
    glVertexAttribPointer(
        uv_id, 2, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<const void*>(sizeof(float) * 6 * vertices_count));
    glVertexAttribPointer(
        tangent_id, 3, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<const void*>(sizeof(float) * 8 * vertices_count));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glUniformMatrix4fv(model_id, 1, GL_TRUE, glm::value_ptr(model));
    glUniformMatrix4fv(
        rotation_id, 1, GL_TRUE,
        glm::value_ptr(rotate_matrix));  // TODO add model model_rotation

    glEnableVertexAttribArray(position_id);
    glEnableVertexAttribArray(normal_id);
    glEnableVertexAttribArray(uv_id);
    glEnableVertexAttribArray(tangent_id);
    glDrawElements(GL_TRIANGLES, faces_count * 3, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(position_id);
    glDisableVertexAttribArray(normal_id);
    glDisableVertexAttribArray(uv_id);
    glDisableVertexAttribArray(tangent_id);
}

void Mesh::SetVectorRotate(glm::vec3 v, float phi) {
    rotation_angle = phi;
    rotation_vector = v;
}

int Mesh::GetFacesCount() { return faces_count; }

int Mesh::GetVerticesCount() { return vertices_count; }

void Mesh::SetTexture(GLuint textureUnit) { shadowMap = textureUnit; }

/*shared_ptr<Shader> Mesh::GetShader()
{
    return shaderProgram;
}
void Mesh::SetShader(shared_ptr<Shader> shader)
{
    shaderProgram = shader;
}*/
