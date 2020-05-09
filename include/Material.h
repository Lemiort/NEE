#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H
#include <GL/glew.h>

#include <map>
#include <memory>

#include "Shader.h"
#include "Texture.h"

class Material {
public:
    using TextureNumber = GLuint;
    Material();
    virtual ~Material() = default;
    bool Init(std::shared_ptr<Shader> _sh);
    void Use();
    void SetColorTexture(std::shared_ptr<Texture2D> _colorMap);
    void SetNormalTexture(std::shared_ptr<Texture2D> _normalMap);
    void SetSpecularTexture(std::shared_ptr<Texture2D> _specularMap);
    void SetShadowTexture(std::shared_ptr<Texture2D> _shadowMap);
    void SetTexture(std::shared_ptr<Texture2D> map, TextureNumber num);
    void SetTexture(GLuint map, TextureNumber num);
    std::shared_ptr<Shader> GetShader() { return shaderProgram; }

protected:
    std::shared_ptr<Shader> shaderProgram;
    std::shared_ptr<Texture2D> colorMap, normalMap, specularMap;
    std::shared_ptr<Texture2D> shadowMap;
    std::shared_ptr<AbstractTexture> abstractMap;
    GLuint colTexID, texBufferID, normBufferID, normSamplerID;
    GLuint specBufferID, specSamplerID;
    std::map<TextureNumber, GLuint> abstractSamplersID;
    GLuint shadowSamplerID;
    std::map<TextureNumber, GLuint> texturesID;
};

#endif  // INCLUDE_MATERIAL_H
