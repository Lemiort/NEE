#ifndef TEXT_2D_H_INCLUDED
#define TEXT_2D_H_INCLUDED
#include <glad/gl.h>

#include <cstdint>
#include <cstring>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "Camera.hpp"
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
 GLint sverticesID{}, spositionID{};
 GLint uvID{}, suvID{};
 GLint sizeID{};
 GLint texSamplerID{};
 GLuint texBufferID{};
 GLint colorID{};
 glm::vec4 color;
 std::string filename;
 float aratio{1};

 // ratio of image sizes to screen
 float kx{}, ky{};

 // pixel size in UV coordinates
 float pkx{}, pky{};
 uint32_t fontHeight{};
 uint32_t imageWidth{}, imageHeight{};
 std::string fontName;
 std::map<char16_t, FontCharacter> fontInfo;

 float realWidth{};
 float realHeight{};
 float dx{};
 float xOffset{};
 float yOffset{};
 FontCharacter currentCharacter{};

protected:
    glm::vec2 characterLength;
    char16_t character{};
    std::map<KerningPair, float> kerningInfo;

public:
    Font2d();
    ~Font2d() override;
    bool Init(std::string filename, std::shared_ptr<Shader> _sh);
    [[nodiscard]] uint32_t GetFontHeight() const;
    [[nodiscard]] float GetWidth(char16_t c) const;
    [[nodiscard]] float GetHeight(char16_t c) const;
    [[nodiscard]] float GetSpaceWidth() const;
    void SetAspectRatio(int width, int height);
    void SetAspectRatio(float /*f*/);
    void SetCharacter(char16_t c);

    // returns the length of the current character
    glm::vec2 GetLastCharacterLength();
    void Render(const Camera& cam) override;
    friend class FontLine2d;
};

// font line
class FontLine2d : public RenderableObject, public PlaceableObject {
private:
    Font2d character;
    std::shared_ptr<Shader> shaderProgram;
    float prevX{}, prevY{};
    float aratio{};
    float spaceWidth{};
    char16_t prevChar{};
    std::string text;

public:
    FontLine2d();
    ~FontLine2d() override;
    bool Init(std::string filename, std::shared_ptr<Shader> _sh);
    void SetAspectRatio(int w, int h);
    void SetText(std::string _text);
    // void Render(string text,float x, float y, float size);
    void Render(const Camera& cam) override;
};

#endif  // TEXT_2D_H_INCLUDED
