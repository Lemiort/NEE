#include "Mesh.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/transform.hpp>

Mesh::Mesh() {
    // mat=NULL;
    shadowMap = 0;
    rPhi = 0;
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
    // находим позиции uniform-переменных
    model_id = shaderProgram->GetUniformLocation("model");
    rotation_id = shaderProgram->GetUniformLocation(
        "model_rotation");  // вращение объекта
}

bool Mesh::Init(std::shared_ptr<Material> _mat, const char* model) {
    mat = _mat;
    shaderProgram = mat->GetShader();
    // mat->Use();

    if (shaderProgram == NULL) return false;

    Scale = 0;
    int* spindices = NULL;
    float* spvertices = NULL;
    float* spnormals = NULL;
    float* spuvs = NULL;
    float* sptangent = NULL;
    spfaces = 0;
    spverts = 0;
    try {
        FILE* fp;
        std::fstream fout("ImporObj.log", std::ios::out);
        // fopen_s(&fp,model,"r+b");
        fp = fopen(model, "r+b");
        if (!fp) return false;
        fread(&spverts, sizeof(int), 1, fp);
        fout << spverts << std::endl;
        spvertices = new float[spverts * 3];
        spuvs = new float[spverts * 2];
        spnormals = new float[spverts * 3];
        sptangent = new float[spverts * 3];
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
            fread(&sptangent[i], sizeof(float), 1, fp);
        }
        for (int i = 0; i < spverts; i++) {
            fout << i << " ";
            fout << spvertices[3 * i] << ",";
            fout << spvertices[3 * i + 1] << ",";
            fout << spvertices[3 * i + 2] << ",";
            fout << spuvs[2 * i + 0] << ",";
            fout << spuvs[2 * i + 1] << ",";
            fout << spnormals[3 * i + 0] << ",";
            fout << spnormals[3 * i + 1] << ",";
            fout << spnormals[3 * i + 2] << ",\n";
        }
        // vertices[0]=f;
        fread(&spfaces, sizeof(int), 1, fp);
        fout << "\n" << spfaces << std::endl;
        spindices = new int[spfaces * 3];
        for (int i = 0; i < spfaces; i++) {
            fread(&spindices[3 * i], sizeof(int), 1, fp);
            fread(&spindices[3 * i + 1], sizeof(int), 1, fp);
            fread(&spindices[3 * i + 2], sizeof(int), 1, fp);
        }
        fclose(fp);
        for (int i = 0; i < spfaces; i++) {
            fout << spindices[3 * i] << ",";
            fout << spindices[3 * i + 1] << "," << spindices[3 * i + 2]
                 << std::endl;
        }
        fout.close();
    } catch (const std::bad_alloc&) {
        printf("\nError creating make_shared<Mesh>");
        return false;
    }

    // создаём буффер, в котором будем хранить всё
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // создаём буффер
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (3 + 3 + 2 + 3) * (spverts),
                 nullptr, GL_STATIC_DRAW);
    // загружаем вершины в буффер
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * spverts,
                    spvertices);
    // нормали
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 3 * spverts,
                    sizeof(float) * 3 * spverts, spnormals);
    // текстурные координаты
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 6 * spverts,
                    sizeof(float) * 2 * spverts, spuvs);
    // тангент
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 8 * spverts,
                    sizeof(float) * 3 * spverts, sptangent);

    // привязываем индексы к буфферу
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * spfaces, spindices,
                 GL_STATIC_DRAW);

    position_id = shaderProgram->GetAttribLocation("vertex_position");
    normal_id = shaderProgram->GetAttribLocation("vertex_normal");
    uv_id = shaderProgram->GetAttribLocation("vertex_uv");
    tangent_id = shaderProgram->GetAttribLocation("vertex_tangent");
    // находим позиции uniform-переменных
    model_id = shaderProgram->GetUniformLocation("model");
    rotation_id = shaderProgram->GetUniformLocation(
        "model_rotation");  // вращение объекта

    for (int i = 0; i < 3; i++) {
        position[i] = 0;
        model_rotation[i] = 0;
        scale[i] = 1;
    }

    // убираем за собой
    delete[] spvertices;
    delete[] spuvs;
    delete[] spnormals;
    delete[] sptangent;

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
        reinterpret_cast<const void*>(sizeof(float) * 3 * spverts));
    glVertexAttribPointer(
        uv_id, 2, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<const void*>(sizeof(float) * 6 * spverts));
    glVertexAttribPointer(
        tangent_id, 3, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<const void*>(sizeof(float) * 8 * spverts));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glUniformMatrix4fv(model_id, 1, GL_TRUE, glm::value_ptr(model));
    glUniformMatrix4fv(
        rotation_id, 1, GL_TRUE,
        glm::value_ptr(rotate_matrix));  // TODO add model model_rotation

    glEnableVertexAttribArray(position_id);
    glEnableVertexAttribArray(normal_id);
    glEnableVertexAttribArray(uv_id);
    glEnableVertexAttribArray(tangent_id);
    glDrawElements(GL_TRIANGLES, spfaces * 3, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(position_id);
    glDisableVertexAttribArray(normal_id);
    glDisableVertexAttribArray(uv_id);
    glDisableVertexAttribArray(tangent_id);
}

void Mesh::SetVectorRotate(glm::vec3 v, float phi) {
    rPhi = phi;
    rv = v;
}

int Mesh::GetNumFaces() { return spfaces; }

int Mesh::GetNumVerts() { return spverts; }

void Mesh::SetTexture(GLuint textureUnit) { shadowMap = textureUnit; }

/*shared_ptr<Shader> Mesh::GetShader()
{
    return shaderProgram;
}
void Mesh::SetShader(shared_ptr<Shader> shader)
{
    shaderProgram = shader;
}*/
