#ifndef TEXT_2D_H_INCLUDED
#define TEXT_2D_H_INCLUDED
#include <glad/gl.h>

#include <cstdint>
#include <cstring>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <memory>
#include <string>
#include <utility>

#include "Camera.hpp"
#include "FontMetrics.hpp"
#include "RenderableObject.hpp"
#include "Shader.hpp"
#include "TextLayout.hpp"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

// font
class Font2d : public RenderableObject {
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
    FontMetrics metrics_;

    float realWidth{};
    float realHeight{};
    float dx{};
    float xOffset{};
    float yOffset{};
    FontCharacter currentCharacter{};
    TextLayout layout_{};

protected:
    glm::vec2 characterLength;
    char16_t character{};

public:
    Font2d();
    ~Font2d() override;
    bool Init(std::string filename, std::shared_ptr<Shader> _sh);
    [[nodiscard]] uint32_t GetFontHeight() const;
    [[nodiscard]] float GetWidth(char16_t c) const;
    [[nodiscard]] float GetHeight(char16_t c) const;
    [[nodiscard]] float GetSpaceWidth() const;
    [[nodiscard]] float GetKerning(char16_t previous,
                                   char16_t current) const noexcept;
    void SetAspectRatio(int width, int height);
    void SetAspectRatio(float /*f*/);
    void SetPosition(const glm::vec2& position) noexcept;
    void SetFontSize(float fontSizePixels) noexcept;
    void SetCharacter(char16_t c);

    // returns the length of the current character
    glm::vec2 GetLastCharacterLength();
    void Render(const Camera& cam) override;
    friend class FontLine2d;
};

// font line
class FontLine2d : public RenderableObject {
private:
    Font2d character;
    std::shared_ptr<Shader> shaderProgram;
    float prevX{}, prevY{};
    float aratio{};
    float spaceWidth{};
    char16_t prevChar{};
    std::string text;
    TextLayout layout_{};

public:
    FontLine2d();
    ~FontLine2d() override;
    bool Init(std::string filename, std::shared_ptr<Shader> _sh);
    void SetAspectRatio(int w, int h);
    void SetText(std::string _text);
    void SetPosition(float x, float y) noexcept;
    void SetPosition(const glm::vec2& position) noexcept;
    [[nodiscard]] const glm::vec2& GetPosition() const noexcept;
    void SetFontSize(float fontSizePixels) noexcept;
    [[nodiscard]] float GetFontSize() const noexcept;
    // void Render(string text,float x, float y, float size);
    void Render(const Camera& cam) override;
};

#endif  // TEXT_2D_H_INCLUDED
