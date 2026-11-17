#ifndef MATERIAL_H
#define MATERIAL_H
#include <glad/gl.h>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <vector>

#include "Shader.hpp"
#include "Texture.hpp"

class Material {
public:
    explicit Material(gsl_lite::not_null<std::shared_ptr<Shader>> shader);
    virtual ~Material();
    void Use(float drawOpacity = 1.0F) const;
    void UseWithShader(Shader& shader, float drawOpacity = 1.0F) const;
    void SetColorTexture(std::shared_ptr<Texture2D> _colorMap);
    void SetNormalTexture(std::shared_ptr<Texture2D> _normalMap);
    void SetSpecularTexture(std::shared_ptr<Texture2D> _specularMap);
    void SetShadowTexture(std::shared_ptr<Texture2D> _shadowMap);
    // Custom texture units borrow handles; their owners must outlive use.
    void SetTexture(const std::shared_ptr<Texture2D>& _map, GLuint num);
    void SetTexture(GLuint _map, GLuint num);
    // Light the surface gives off by itself, added to the lit color whatever
    // the scene's lights are: the emissive texture times the emissive color.
    // The texture says where the surface glows and the color how strongly.
    // By default the texture is white and the color black, so nothing glows;
    // a color alone makes the whole surface glow.
    void SetEmissiveColor(const glm::vec3& color) { emissiveColor_ = color; }
    [[nodiscard]] const glm::vec3& GetEmissiveColor() const noexcept {
        return emissiveColor_;
    }
    void SetEmissiveTexture(std::shared_ptr<Texture2D> _emissiveMap);
    // Texture coordinates from the world position instead of the mesh,
    // `scale` tiles per world unit: walls facing x use (z, y), walls facing
    // z use (x, y), floors and roofs (x, z). A texture then keeps its size
    // on every face whatever the mesh's scale, like windows on buildings of
    // any height. (0, 0), the default, uses the mesh's coordinates. Normal
    // maps still follow the mesh's tangents.
    void SetWorldTextureScale(const glm::vec2& scale) {
        worldTextureScale_ = scale;
    }
    [[nodiscard]] const glm::vec2& GetWorldTextureScale() const noexcept {
        return worldTextureScale_;
    }
    // The emissive texture takes the last texture unit, so custom units
    // (SetTexture) run from 4 up to, but not including, this one.
    [[nodiscard]] GLuint GetEmissiveTextureUnit() const noexcept {
        return static_cast<GLuint>(max_texture_units - 1);
    }
    [[nodiscard]] std::shared_ptr<Shader> GetShader() const {
        return shaderProgram;
    }

protected:
    void BindTextures() const;
    gsl_lite::not_null<std::shared_ptr<Shader>> shaderProgram;
    std::shared_ptr<Texture2D> colorMap, normalMap, specularMap;
    std::shared_ptr<Texture2D> shadowMap;
    std::shared_ptr<Texture2D> emissiveMap;
    GLint colTexID{}, normSamplerID{};
    GLuint texBufferID{}, normBufferID{};
    GLuint specBufferID{};
    GLint specSamplerID{};
    std::vector<GLint> abstractSamplersID;
    GLint shadowSamplerID{};
    GLint opacityID{-1};
    GLint emissiveID{-1};
    GLint emissiveSamplerID{-1};
    glm::vec3 emissiveColor_{0.0F};
    GLint worldTextureScaleID{-1};
    glm::vec2 worldTextureScale_{0.0F};
    GLint max_texture_units{};
    std::vector<GLuint> texturesID;
};

#endif  // MATERIAL_H
