#ifndef MATERIAL_H
#define MATERIAL_H
#include <glad/gl.h>

#include <memory>
#include <vector>

#include "Shader.hpp"
#include "Texture.hpp"

class Material {
public:
    Material();
    virtual ~Material();
    bool Init(const std::shared_ptr<Shader>& _sh);
    void Use();
    void SetColorTexture(std::shared_ptr<Texture2D> _colorMap);
    void SetNormalTexture(std::shared_ptr<Texture2D> _normalMap);
    void SetSpecularTexture(std::shared_ptr<Texture2D> _specularMap);
    void SetShadowTexture(std::shared_ptr<Texture2D> _shadowMap);
    void SetTexture(const std::shared_ptr<Texture2D>& _map, GLuint num);
    void SetTexture(GLuint _map, GLuint num);
    std::shared_ptr<Shader> GetShader() { return shaderProgram; }

protected:
    std::shared_ptr<Shader> shaderProgram;
    std::shared_ptr<Texture2D> colorMap, normalMap, specularMap;
    std::shared_ptr<Texture2D> shadowMap;
    std::shared_ptr<AbstractTexture> abstractMap;
    GLuint colTexID{}, texBufferID{}, normBufferID{}, normSamplerID{};
    GLuint specBufferID{}, specSamplerID{};
    std::vector<GLuint> abstractSamplersID;
    // GLuint colSamplerUI, normSamplerUI;
    GLuint shadowSamplerID{};
    GLint max_texture_units{};
    std::vector<GLuint> texturesID;
};

#endif  // MATERIAL_H
