#include "Text_2D.h"

#include <array>
#include <fstream>
#include <sstream>
#include <stdexcept>  // std::out_of_range

FontLine2d::FontLine2d() { shaderProgram = nullptr; }

FontLine2d::~FontLine2d() {}

bool FontLine2d::Init(std::string filename, std::shared_ptr<Shader> _sh) {
    return character.Init(filename, _sh);
}

void FontLine2d::SetAspectRatio(int w, int h) {
    character.SetAspectRatio(w, h);
    aspect_ratio = static_cast<float>(h) / static_cast<float>(w);
}

void FontLine2d::SetText(std::string _text) { text = _text; }

// void FontLine2d::Render(string text, float startX, float startY,float size )
void FontLine2d::Render(const Camera& cam) {
    float dx = 0.0f;
    prevChar = 0;
    // float dy=0.0f;
    glm::vec2 temp;
    spaceWidth = character.GetSpaceWidth() /
                 static_cast<float>(character.GetFontHeight());
    for (unsigned int i = 0; i < text.length(); ++i) {
        // get character info
        uint32_t code = (((uint32_t)prevChar) << 16) | ((uint32_t)text.at(i));
        float kerning = 0;
        try {
            kerning = character.kerningInfo.at(code);
        } catch (std::out_of_range) {
            kerning = 0;
        }
        dx += kerning;

        // add space if there is no
        if ((unsigned int)text.at(i) == (unsigned int)' ') {
            dx += position[2] * spaceWidth;
        }

        // rendering
        character.SetPosition(position[0] + dx, position[1], position[2]);
        character.SetCharacter(text.at(i));
        temp = character.GetLastCharacterLength();
        character.Render(cam);

        // if last character  exists
        if (temp.x > 0.0f)
            // add offset abd character divider
            dx += temp.x + position[2] * spaceWidth / 4.0f;
        else
            continue;
        // remember last character
        prevChar = (unsigned int)text.at(i);
    }
}

Font2d::Font2d() {
    color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    aspect_ratio = 1;
}

Font2d::~Font2d() {}

float Font2d::GetHeight(unsigned int c) {
    FontCharacter temp = fontInfo.at(c);
    float realHeight =
        static_cast<float>(temp.height) / static_cast<float>(imageHeight);
    float dx = 1.0f;
    return (2 * dx) * realHeight * ky;
}

float Font2d::GetWidth(unsigned int c) {
    FontCharacter temp = fontInfo.at(c);
    float realWidth =
        static_cast<float>(temp.width) / static_cast<float>(imageWidth);
    float dx = 1.0f;
    return (2 * dx) * realWidth * kx;
}

float Font2d::GetSpaceWidth() {
    FontCharacter temp = fontInfo.at(34);
    float realWidth =
        static_cast<float>(temp.origW) / static_cast<float>(imageWidth);
    float dx = 1.0f;
    return (2 * dx) * realWidth * kx;
}

int Font2d::GetFontHeight() { return fontHeight; }

bool Font2d::Init(std::string _filename, std::shared_ptr<Shader> _sh) {
    shaderProgram = _sh;
    filename = _filename;
    std::string fntFilename = _filename;
    std::string imgFilename;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    vertexes_id = shaderProgram->GetAttribLocation("Position");
    uv_id = shaderProgram->GetAttribLocation("UV");
    positions_id = shaderProgram->GetUniformLocation("s_Position");
    suvID = shaderProgram->GetUniformLocation("s_UV");
    colorID = shaderProgram->GetUniformLocation("textColor");

    // fill indexes
    std::array<int, 4> indicies{0, 1, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * std::size(indicies),
                 indicies.data(), GL_STATIC_DRAW);

    // fillling font info
    std::fstream fin;
    fin.open(filename.c_str(), std::ios::in);
    // temp var for reading
    std::string in_s;
    bool kerning = false;
    bool data = false;
    while (1) {
        getline(fin, in_s);
        if (!fin.eof()) {
            // character data reading
            if (data) {
                std::stringstream sstr;
                unsigned int t1[6];
                int t2[2];
                sstr << in_s;
                unsigned int code;
                sstr >> code;
                sstr >> t1[0] >> t1[1] >> t1[2] >> t1[3] >> t2[0] >> t2[1] >>
                    t1[4] >> t1[5];
                FontCharacter temp2 = FontCharacter(t1[0], t1[1], t1[2], t1[3],
                                                    t2[0], t2[1], t1[4], t1[5]);
                fontInfo.insert(
                    std::pair<unsigned int, FontCharacter>(code, temp2));
            }
            // kerning data reading
            if (kerning) {
                std::stringstream sstr;
                sstr << in_s;
                uint16_t code1;
                sstr >> code1;
                uint16_t code2;
                float f1;
                sstr >> code2 >> f1;
                uint32_t code = ((uint32_t)code1 << 16) | ((uint32_t)code2);
                kerningInfo.insert(std::pair<uint32_t, float>(code, f1));
            }
            // find texture name
            int t = in_s.find("textures: ");
            std::string temp("textures: ");
            if (t == 0) {
                imgFilename =
                    std::string("fonts/") + std::string(in_s, temp.length());
                printf("\nFont image is %s", imgFilename.c_str());
            }

            // find font name
            t = in_s.find("px");
            if (t >= 0) {
                int t2 = in_s.find(" ");
                fontName = std::string(in_s, 0, t2);
                std::stringstream sstr;
                temp = std::string(in_s, t2, t - t2);
                sstr << temp;
                sstr >> fontHeight;
                data = true;
                printf("\nFont name is %s", fontName.c_str());
                printf("\nFont height is %d", fontHeight);
            }
            // fin kerning section
            t = in_s.find("kerning pairs:");
            if (t >= 0) {
                // stop characters reading
                data = false;
                // start kerning reading
                kerning = true;
            }

        } else {
            break;
        }
        in_s.clear();
    }

    texBufferID = SOIL_load_OGL_texture(
        imgFilename.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB |
            SOIL_FLAG_COMPRESS_TO_DXT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    texSamplerID = shaderProgram->GetUniformLocation("texSampler");

    // get size from pn header
    FILE* imageFile = fopen(imgFilename.c_str(), "rb");
    char buffer2[4];
    fread(&buffer2, 1, 4, imageFile);
    fread(&buffer2, 1, 4, imageFile);
    fread(&buffer2, 1, 4, imageFile);
    fread(&buffer2, 1, 4, imageFile);

    // parse width
    fread(&buffer2, 1, 4, imageFile);
    imageWidth = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
                 ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] << 24);
    printf("\n width=%d", imageWidth);

    // parse height
    fread(&buffer2, 1, 4, imageFile);
    imageHeight = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
                  ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] << 24);
    printf("\n height=%d", imageHeight);

    // pixels to uv
    pkx = 1.0f / static_cast<float>(imageWidth);
    pky = 1.0f / static_cast<float>(imageHeight);

    fclose(imageFile);

    return true;
}

void Font2d::SetAspectRatio(int _width, int _height) {
    aspect_ratio = static_cast<float>(_height) / static_cast<float>(_width);
    kx = static_cast<float>(imageWidth) / static_cast<float>(_width);
    ky = static_cast<float>(imageHeight) / static_cast<float>(_height);
}

void Font2d::SetAspectRatio(float f) { aspect_ratio = f; }

void Font2d::SetCharacter(unsigned int c) {
    character = c;

    // set y position
    position[2] = position[2] / static_cast<float>(fontHeight);
    temp = FontCharacter(0, 0, 0, 0, 0, 0, 0, 0);
    try {
        temp = fontInfo.at(character);
    } catch (const std::out_of_range& oor) {
        // printf("\n char is out of range");
        characterLength = glm::vec2(-1.0f, -1.0f);
        return;
    }
    realWidth = static_cast<float>(temp.width) / static_cast<float>(imageWidth);
    realHeight =
        static_cast<float>(temp.height) / static_cast<float>(imageHeight);
    dx = 1.0f;
    xOffset = position[2] * (2 * dx) * kx * static_cast<float>(temp.xOffset) /
              static_cast<float>(imageWidth);
    yOffset = position[2] * (-2 * dx) * ky * static_cast<float>(temp.yOffset) /
              static_cast<float>(imageHeight);

    characterLength = glm::vec2((2 * dx) * realWidth * kx * position[2],
                                (2 * dx) * realHeight * ky);
}

glm::vec2 Font2d::GetLastCharacterLength() {
    // return glm::vec2((2*dx)*realWidth*kx*size,(2*dx)*realHeight*ky);
    return characterLength;
}

// glm::vec2 Font2d::Render(unsigned int c,float px,float py,float size)
void Font2d::Render(const Camera& cam) {
    SetAspectRatio(cam.GetWidth(), cam.GetHeight());
    shaderProgram->Use();

    float vertices[] = {0.0f,
                        (-2 * dx) * realHeight * ky,
                        0.0f,
                        0.0f,
                        (2 * dx) * realWidth * kx,
                        (-2 * dx) * realHeight * ky,
                        (2 * dx) * realWidth * kx,
                        0.0f};
    /*float vertices[]={0.0f,0.0f,
                        0.0f,1.0f,
                        1.0f,0.0f,
                        1.0f,1.0f
                            };*/
    float uvs[] = {0.0f,      -realHeight, 0.0f,      0.0f,
                   realWidth, -realHeight, realWidth, 0.0f};

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(uvs), nullptr,
                 GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(vertices), &uvs);

    vertexes_id = shaderProgram->GetAttribLocation("Position");
    uv_id = shaderProgram->GetAttribLocation("UV");
    positions_id = shaderProgram->GetUniformLocation("s_Position");
    suvID = shaderProgram->GetUniformLocation("s_UV");
    sizeID = shaderProgram->GetUniformLocation("size");
    colorID = shaderProgram->GetUniformLocation("textColor");
    float u =
        static_cast<float>(temp.x_position) / static_cast<float>(imageWidth);
    float v = 1.0f - static_cast<float>(temp.y_position) /
                         static_cast<float>(imageHeight);

    // screen position vector
    glUniform2f(positions_id, position[0] + xOffset, position[1] + yOffset);
    // uv offset
    glUniform2f(suvID, u, v);
    // size
    glUniform1f(sizeID, position[2]);

    glVertexAttribPointer(vertexes_id, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(uv_id, 2, GL_FLOAT, GL_FALSE, 0,
                          reinterpret_cast<const void*>(sizeof(float) * 8));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    glUniform1i(texSamplerID,
                0);  // set texture as GL_TEXTURE0
    glUniform4f(colorID, color.r, color.g, color.b, color.a);
    glEnableVertexAttribArray(vertexes_id);
    glEnableVertexAttribArray(uv_id);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(vertexes_id);
    glDisableVertexAttribArray(uv_id);
    // return glm::vec2((2*dx)*realWidth*kx*position[2],(2*dx)*realHeight*ky);
}
