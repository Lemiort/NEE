#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <glad/gl.h>

#include <string>
class Texture {
protected:
    GLuint texBufferID;
    bool del;
    std::string ConvertInternalFormatToString(GLenum format);

public:
    GLuint GetTextureID() const;
    explicit Texture(bool _del = true);
    virtual ~Texture();
    // virtual ~Texture(bool del=true);
    std::string GetParameters();
    virtual void Bind(GLenum TextureUnit);
};

class Texture2D : public Texture {
protected:
public:
    // abstract texture constructor
    explicit Texture2D(GLuint texBufferID, bool _del = true);

    // default constructor, deletes the texture after use
    Texture2D() = default;

    ~Texture2D() override;
    virtual bool Load(const char* filename);
};

class ShadowTexture : public Texture2D {
private:
public:
    ShadowTexture();
    ~ShadowTexture() override;
    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
};

class AbstractTexture : public Texture {
public:
    AbstractTexture() { texBufferID = 0; }
    ~AbstractTexture() override = default;

    void SetTexture(GLuint _texID);

protected:
};

#endif  // TEXTURE_H_INCLUDED
