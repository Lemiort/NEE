#ifndef MATERIAL_H
#define MATERIAL_H
#include <glad/gl.h>

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
    void SetColorTexture(std::shared_ptr<Texture2D> _colorMap);
    void SetNormalTexture(std::shared_ptr<Texture2D> _normalMap);
    void SetSpecularTexture(std::shared_ptr<Texture2D> _specularMap);
    void SetShadowTexture(std::shared_ptr<Texture2D> _shadowMap);
    void SetTexture(const std::shared_ptr<Texture2D>& _map, GLuint num);
    void SetTexture(GLuint _map, GLuint num);
    [[nodiscard]] std::shared_ptr<Shader> GetShader() const {
        return shaderProgram;
    }

protected:
    gsl_lite::not_null<std::shared_ptr<Shader>> shaderProgram;
    std::shared_ptr<Texture2D> colorMap, normalMap, specularMap;
    std::shared_ptr<Texture2D> shadowMap;
    std::shared_ptr<AbstractTexture> abstractMap;
    GLint colTexID{}, normSamplerID{};
    GLuint texBufferID{}, normBufferID{};
    GLuint specBufferID{};
    GLint specSamplerID{};
    std::vector<GLint> abstractSamplersID;
    // GLuint colSamplerUI, normSamplerUI;
    GLint shadowSamplerID{};
    GLint opacityID{-1};
    GLint max_texture_units{};
    std::vector<GLuint> texturesID;
};

#endif  // MATERIAL_H
