#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED
#include <glad/gl.h>

#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <string>

#include "Camera.hpp"
#include "Material.hpp"
#include "MaterialObject.hpp"
#include "Transformable.hpp"

struct Vertex {
    glm::vec3 Pos;
    glm::vec3 UV;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    Vertex() = default;
    Vertex(glm::vec3& pos, glm::vec3& uv, glm::vec3& normal, glm::vec3& tangent)
        : Pos(pos), UV(uv), Normal(normal), Tangent(tangent) {}
};

struct MeshShaderBindings {
    GLint position{};
    GLint normal{};
    GLint uv{};
    GLint tangent{};
    GLint world{};
    GLint rotation{};
    GLint instanceWorld{-1};
    GLint instanceRotation{-1};
    GLint instanced{-1};

    [[nodiscard]] static MeshShaderBindings FromShader(const Shader& shader);
};

class Mesh : public MaterialObject, public Transformable {
public:
    Mesh(gsl_lite::not_null<std::shared_ptr<Material>> material,
         const std::string& model);
    ~Mesh() override;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;
    [[nodiscard]] int GetNumFaces() const;
    [[nodiscard]] int GetNumVerts() const;
    void SetMaterial(std::shared_ptr<Material> material) override;
    void Render(const Camera& cam) override;
    void Draw(const Transform& transform, const Material& material,
              const MeshShaderBindings& bindings, float opacity = 1.0F,
              GLuint instanceBuffer = 0, GLsizei instanceCount = 1) const;

private:
    int faceCount_{};
    int vertexCount_{};
};
#endif  // MESH_H_INCLUDED
