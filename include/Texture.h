#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <GL/glew.h>
#include <SOIL2/SOIL2.h>

#include <sstream>
#include <string>
class Texture {
protected:
    GLuint texBufferID;
    bool del;
    std::string ConvertInternalFormatToString(GLenum format);

public:
    GLuint GetTextureID();
    Texture(bool _del = true);
    virtual ~Texture();
    // virtual ~Texture(bool del=true);
    std::string GetParameters();
    virtual void Bind(GLenum TextureUnit);
};

class Texture2D : public Texture {
protected:
public:
    //конструктор абстрактной текстуры
    Texture2D(GLuint texBufferID, bool _del = true);

    //конструктор по умолчанию, удаляет текстуру после использования
    Texture2D() {}
    virtual ~Texture2D();
    virtual bool Load(const char* filename);
};

class ShadowTexture : public Texture2D {
private:
public:
    ShadowTexture();
    virtual ~ShadowTexture();
    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
};

class AbstractTexture : public Texture {
public:
    AbstractTexture() { texBufferID = 0; }
    ~AbstractTexture() {}
    void SetTexture(GLuint _texID);

protected:
};

#endif  // TEXTURE_H_INCLUDED
