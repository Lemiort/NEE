#ifndef MATERIAL_H
#define MATERIAL_H
#include <GL/glew.h>
#include <Shader.h>
#include <Texture.h>
#include <memory>

class Material {
public:
    Material();
    virtual ~Material();
    bool Init(shared_ptr<Shader> _sh);
    void Use();
    void SetColorTexture(shared_ptr<Texture2D> _colorMap);
    void SetNormalTexture(shared_ptr<Texture2D> _normalMap);
    void SetSpecularTexture(shared_ptr<Texture2D> _specularMap);
    void SetShadowTexture(shared_ptr<Texture2D> _shadowMap);
    void SetTexture(shared_ptr<Texture2D> _map, GLuint num);
    void SetTexture(GLuint _map, GLuint num);
    shared_ptr<Shader> GetShader() { return shaderProgram; }

protected:
    shared_ptr<Shader> shaderProgram;
    shared_ptr<Texture2D> colorMap, normalMap, specularMap;
    shared_ptr<Texture2D> shadowMap;
    shared_ptr<AbstractTexture> abstractMap;
    GLuint colTexID, texBufferID, normBufferID, normSamplerID;
    GLuint specBufferID, specSamplerID;
    GLuint* abstractSamplersID;
    // GLuint colSamplerUI, normSamplerUI;
    GLuint shadowSamplerID;
    GLint max_texture_units;
    GLuint* texturesID;
};

#endif  // MATERIAL_H
