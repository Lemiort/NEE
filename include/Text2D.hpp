#ifndef TEXT_2D_H_INCLUDED
#define TEXT_2D_H_INCLUDED
#include <glad/gl.h>

#include <cstring>
#include <map>

#include "Math3d.hpp"
#include "PlaceableObject.hpp"
#include "RenderableObject.hpp"
#include "Shader.hpp"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

struct FontCharacter {
    uint32_t xpos;
    uint32_t ypos;
    uint32_t width;
    uint32_t height;
    int32_t xOffset;
    int32_t yOffset;
    uint32_t origW;
    uint32_t origH;
};
using KerningPair = std::pair<char16_t, char16_t>;

// font
class Font2d : public RenderableObject, public PlaceableObject {
private:
    GLuint sverticesID, spositionID;
    GLuint uvID, suvID;
    GLuint sizeID;
    GLuint texSamplerID, texBufferID;
    GLuint colorID;
    Vector4f color;
    std::string filename;
    float aratio;

    // ratio of image sizes to screen
    float kx, ky;

    // pixel size in UV coordinates
    float pkx, pky;
    uint32_t fontHeight;
    uint32_t imageWidth, imageHeight;
    std::string fontName;
    std::map<char16_t, FontCharacter> fontInfo;

    float realWidth;
    float realHeight;
    float dx;
    float xOffset;
    float yOffset;
    FontCharacter currentCharacter;

protected:
    Vector2f characterLength;
    char16_t character;
    std::map<KerningPair, float> kerningInfo;

public:
    Font2d();
    ~Font2d();
    bool Init(std::string filename, std::shared_ptr<Shader> _sh);
    uint32_t GetFontHeight() const;
    float GetWidth(char16_t c) const;
    float GetHeight(char16_t c) const;
    float GetSpaceWidth() const;
    void SetAspectRatio(int width, int height);
    void SetAspectRatio(float);
    void SetCharacter(char16_t c);

    // returns the length of the current character
    Vector2f GetLastCharacterLength();
    void Render(const Camera& cam);
    friend class FontLine2d;
};

// font line
class FontLine2d : public RenderableObject, public PlaceableObject {
private:
    Font2d character;
    std::shared_ptr<Shader> shaderProgram;
    float prevX, prevY;
    float aratio;
    float spaceWidth;
    char16_t prevChar;
    std::string text;

public:
    FontLine2d();
    ~FontLine2d();
    bool Init(std::string filename, std::shared_ptr<Shader> _sh);
    void SetAspectRatio(int w, int h);
    void SetText(std::string _text);
    // void Render(string text,float x, float y, float size);
    void Render(const Camera& cam);
};

#endif  // TEXT_2D_H_INCLUDED
