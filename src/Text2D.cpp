#include "Text2D.hpp"

#include <glad/gl.h>
#include <math.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <memory>
#include <ostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "Camera.hpp"
#include "Math3d.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "spdlog/spdlog.h"
#include "stb_image.h"

// The shader program is managed via a shared_ptr. Initialize it to nullptr.
FontLine2d::FontLine2d() : shaderProgram(nullptr) {}

FontLine2d::~FontLine2d() = default;

bool FontLine2d::Init(std::string filename, std::shared_ptr<Shader> _sh) {
    return character.Init(std::move(filename), std::move(_sh));
}

void FontLine2d::SetAspectRatio(int w, int h) {
    character.SetAspectRatio(w, h);
    aratio = static_cast<float>(h) / static_cast<float>(w);
}

void FontLine2d::SetText(std::string _text) { text = std::move(_text); }

// void FontLine2d::Render(string text, float startX, float startY,float size )
void FontLine2d::Render(const Camera& cam) {
    float dx = 0.0F;
    prevChar = 0;
    // float dy=0.0f;
    spaceWidth = character.GetSpaceWidth() /
                 (static_cast<float>(character.GetFontHeight()));
    for (size_t i = 0; i < text.length(); ++i) {
        // look up info about this character
        float kerning = 0;
        try {
            kerning = character.kerningInfo.at(
                {prevChar, static_cast<char16_t>(text.at(i))});
        } catch (std::out_of_range) {
            kerning = 0;
        }
        dx += kerning;

        // if we found a space character, draw it
        if (static_cast<char16_t>(text.at(i)) == static_cast<char16_t>(' ')) {
            dx += position[2] * spaceWidth;
        }

        // actual drawing
        character.SetPosition(position[0] + dx, position[1], position[2]);
        character.SetCharacter(text.at(i));
        auto lastCharacterLength = character.GetLastCharacterLength();
        character.Render(cam);

        // if the character exists, draw it
        if (lastCharacterLength.x > 0.0F) {
            // space and separator between characters
            dx += lastCharacterLength.x + (position[2] * spaceWidth / 4.0F);
        } else {
            continue;  // no character - do not write anything
        }
        // remember the previous character
        prevChar = static_cast<char16_t>(text.at(i));
    }
}

Font2d::Font2d() : aratio(1) { color = Vector4f(1.0F, 1.0F, 1.0F, 1.0F); }

Font2d::~Font2d() = default;

float Font2d::GetHeight(char16_t c) const {
    FontCharacter const temp = fontInfo.at(c);
    float const realHeight =
        static_cast<float>(temp.height) / static_cast<float>(imageHeight);
    float const dx = 1.0F;
    return (2 * dx) * realHeight * ky;
}

float Font2d::GetWidth(char16_t c) const {
    FontCharacter const temp = fontInfo.at(c);
    float const realWidth =
        static_cast<float>(temp.width) / static_cast<float>(imageWidth);
    float const dx = 1.0F;
    return (2 * dx) * realWidth * kx;
}

float Font2d::GetSpaceWidth() const {
    FontCharacter const temp = fontInfo.at(34);
    float const realWidth =
        static_cast<float>(temp.origW) / static_cast<float>(imageWidth);
    float const dx = 1.0F;
    return (2 * dx) * realWidth * kx;
}

uint32_t Font2d::GetFontHeight() const { return fontHeight; }

bool Font2d::Init(std::string _filename, std::shared_ptr<Shader> _sh) {
    shaderProgram = _sh;
    filename = _filename;
    std::string imgFilename;
    // generate vertex buffer for future use
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // and index buffer
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    // get shader variables values
    sverticesID = shaderProgram->GetAttribLocation("Position");
    uvID = shaderProgram->GetAttribLocation("UV");
    spositionID = shaderProgram->GetUniformLocation("s_Position");
    suvID = shaderProgram->GetUniformLocation("s_UV");
    colorID = shaderProgram->GetUniformLocation("textColor");

    // fill indices
    const std::array<uint32_t, 4> indicies = {0, 1, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies.data(),
                 GL_STATIC_DRAW);

    /*===========Filling font information============*/
    std::fstream fin;
    fin.open(filename.c_str(), std::ios::in);
    // temporary variable for reading
    std::string in_s;

    enum class FontFileState {
        None = 0,
        TextureFilename,
        FontNameSize,
        FontData,
        KerningAnnouncement,
        KerningData
    };
    FontFileState state = FontFileState::None;

    while (std::getline(fin, in_s)) {
        // read data
        if (state == FontFileState::FontData) {
            std::stringstream sstr;
            sstr << in_s;
            uint16_t code = 0;
            sstr >> code;
            FontCharacter symbol;
            sstr >> symbol.xpos >> symbol.ypos >> symbol.width >>
                symbol.height >> symbol.xOffset >> symbol.yOffset >>
                symbol.origW >> symbol.origH;
            fontInfo.insert({static_cast<char16_t>(code), symbol});
        } else if (state == FontFileState::KerningData) {
            std::stringstream sstr;
            sstr << in_s;
            static_assert(sizeof(uint16_t) == sizeof(char16_t));
            uint16_t code1 = 0;
            sstr >> code1;
            uint16_t code2 = 0;
            float f1 = NAN;
            sstr >> code2 >> f1;
            kerningInfo.insert(
                {{static_cast<char16_t>(code1), static_cast<char16_t>(code2)},
                 f1});
        }
        // find texture name
        std::regex texturesPattern(R"(textures:\s*([a-zA-Z0-9_\-\.]+))");
        std::smatch match;
        if (std::regex_search(in_s, match, texturesPattern)) {
            state = FontFileState::TextureFilename;
            imgFilename = std::string("fonts/") + match[1].str();
            spdlog::info("Font image is \"{}\"", imgFilename);
        }

        // find font name
        std::regex fontNameSizePattern(R"((.*?)\s+(\d+)px$)");
        if (std::regex_search(in_s, match, fontNameSizePattern)) {
            state = FontFileState::FontNameSize;
            fontName = match[1].str();
            fontHeight = std::stoi(match[2].str());

            spdlog::info("Font name is {}", fontName);
            spdlog::info("Font height is {}", fontHeight);

            // flag that data is now being read
            state = FontFileState::FontData;
        }
        // find information about upcoming kerning
        if (in_s.find("kerning pairs:") != in_s.npos) {
            state = FontFileState::KerningData;
        }
    }
    /*================================================*/

    // create texture
    texBufferID = loadTexture(imgFilename);

    // activate texture unit 0
    glActiveTexture(GL_TEXTURE0);
    // bind texture to active texture unit
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    texSamplerID = shaderProgram->GetUniformLocation("texSampler");

    // Get image dimensions
    int width, height, channels;
    if (!stbi_info(imgFilename.c_str(), &width, &height, &channels)) {
        spdlog::error("Error reading image info:{}", stbi_failure_reason());
        return false;
    }
    // Calculate width
    imageWidth = static_cast<uint32_t>(width);
    spdlog::info("width={}", imageWidth);

    // Calculate height
    imageHeight = static_cast<uint32_t>(height);
    spdlog::info("height={}", imageHeight);

    // Convert pixel to relative coordinates
    pkx = 1.0F / static_cast<float>(imageWidth);
    pky = 1.0F / static_cast<float>(imageHeight);

    return true;
}

void Font2d::SetAspectRatio(int _width, int _height) {
    aratio = static_cast<float>(_height) / static_cast<float>(_width);
    kx = static_cast<float>(imageWidth) / static_cast<float>(_width);
    ky = static_cast<float>(imageHeight) / static_cast<float>(_height);
}

void Font2d::SetAspectRatio(float f) { aratio = f; }

void Font2d::SetCharacter(char16_t c) {
    character = c;

    // this is a mess, setting the Y coordinate
    position[2] = position[2] / static_cast<float>(fontHeight);
    currentCharacter = FontCharacter(0, 0, 0, 0, 0, 0, 0, 0);
    try {
        currentCharacter = fontInfo.at(character);
    } catch (const std::out_of_range& oor) {
        // printf("\n char is out of range");
        characterLength = Vector2f(-1.0F, -1.0F);
        return;
    }
    realWidth = static_cast<float>(currentCharacter.width) /
                static_cast<float>(imageWidth);
    realHeight = static_cast<float>(currentCharacter.height) /
                 static_cast<float>(imageHeight);
    dx = 1.0F;
    xOffset = position[2] * (2 * dx) * kx *
              static_cast<float>(currentCharacter.xOffset) /
              static_cast<float>(imageWidth);
    yOffset = position[2] * (-2 * dx) * ky *
              static_cast<float>(currentCharacter.yOffset) /
              static_cast<float>(imageHeight);

    characterLength = Vector2f((2 * dx) * realWidth * kx * position[2],
                               (2 * dx) * realHeight * ky);
}

Vector2f Font2d::GetLastCharacterLength() {
    // return Vector2f((2*dx)*realWidth*kx*size,(2*dx)*realHeight*ky);
    return characterLength;
}

void Font2d::Render(const Camera& cam) {
    SetAspectRatio(cam.GetWidth(), cam.GetHeight());
    shaderProgram->Use();

    float vertices[] = {0.0F,
                        (-2 * dx) * realHeight * ky,
                        0.0F,
                        0.0F,
                        (2 * dx) * realWidth * kx,
                        (-2 * dx) * realHeight * ky,
                        (2 * dx) * realWidth * kx,
                        0.0F};
    /*float vertices[]={0.0f,0.0f,
                        0.0f,1.0f,
                        1.0f,0.0f,
                        1.0f,1.0f
                            };*/
    float uvs[] = {0.0F,      -realHeight, 0.0F,      0.0F,
                   realWidth, -realHeight, realWidth, 0.0F};
    /*  float uvs[]={0.0f,0.0f,
                        0.0f,0.0f,
                        0.0f,0.0f,
                                            0.0f,0.0f};*/
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(uvs), nullptr,
                 GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(vertices), &uvs);

    sverticesID = shaderProgram->GetAttribLocation("Position");
    uvID = shaderProgram->GetAttribLocation("UV");
    spositionID = shaderProgram->GetUniformLocation("s_Position");
    suvID = shaderProgram->GetUniformLocation("s_UV");
    sizeID = shaderProgram->GetUniformLocation("size");
    colorID = shaderProgram->GetUniformLocation("textColor");
    float const u = static_cast<float>(currentCharacter.xpos) /
                    static_cast<float>(imageWidth);
    float const v = 1.0F - (static_cast<float>(currentCharacter.ypos) /
                            static_cast<float>(imageHeight));
    // printf("\nu=%f,  v=%f ",u,v);
    // Calculate offset
    // px+=((float)temp.xOffset/(float)imageWidth)*kx*1.0f*size;
    // py-=((float)temp.yOffset/(float)imageHeight)*ky*1.0f*size;

    // vector position in space
    glUniform2f(spositionID, position[0] + xOffset, position[1] + yOffset);
    // UV offset vector
    glUniform2f(suvID, u, v);
    // Size
    glUniform1f(sizeID, position[2]);

    glVertexAttribPointer(sverticesID, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(uvID, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(float) * 8));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glActiveTexture(GL_TEXTURE0);
    // assign texture to active texture unit
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    glUniform1i(texSamplerID,
                0);  // tell shader to use texture unit 0
    glUniform4f(colorID, color.r, color.g, color.b, color.a);
    glEnableVertexAttribArray(sverticesID);
    glEnableVertexAttribArray(uvID);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(sverticesID);
    glDisableVertexAttribArray(uvID);
    // return Vector2f((2*dx)*realWidth*kx*position[2],(2*dx)*realHeight*ky);
}
