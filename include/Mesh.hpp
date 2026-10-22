#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED
#include <glad/gl.h>

#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <memory>

#include "Camera.hpp"
#include "Material.hpp"
#include "MaterialObject.hpp"
#include "Transformable.hpp"
// #include "Light.hpp"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))
using namespace std;

struct Vertex {
    glm::vec3 Pos;
    glm::vec3 UV;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    Vertex() = default;
    Vertex(glm::vec3& pos, glm::vec3& uv, glm::vec3& normal, glm::vec3& tangent)
        : Pos(pos), UV(uv), Normal(normal), Tangent(tangent) {}
};

class Mesh : public MaterialObject, public Transformable {
public:
protected:
    GLint positionID{}, uvID{}, normalID{}, tangentID{};
    GLint gWorldID{}, gCamViewID{};
    GLint rotateID{};
    GLint dirLightDirID{}, dirLightColID{};
    GLint pointLightColID{}, pointLightIntID{}, pointLightPosID{};
    GLint camtransID{}, camPosID{};
    GLint specBufferID{}, specSamplerID{};
    GLint colSamplerUI{}, normSamplerUI{};
    GLuint shadowMap{};
    int spfaces{}, spverts{};
    int Scale{};

public:
    Mesh();
    ~Mesh() override;
    [[nodiscard]] int GetNumFaces() const;
    [[nodiscard]] int GetNumVerts() const;
    void SetMaterial(shared_ptr<Material> _mat) override;
    bool Init(shared_ptr<Material> _mat, const char* model);
    void SetTexture(GLuint textureUnit);
    void Render(const Camera& cam) override;

private:
};
#endif  // MESH_H_INCLUDED
