#include "Text2D.hpp"

#include <glad/gl.h>
#include <math.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <ostream>
#include <print>
#include <sstream>
#include <stdexcept>  // std::out_of_range]
#include <string>
#include <utility>

#include "Camera.hpp"
#include "Math3d.hpp"
#include "Shader.hpp"
#include "ShaderFunctions.hpp"
#include "stb_image.h"

// The shader program is managed via a shared_ptr. Initialize it to nullptr.
FontLine2d::FontLine2d() : shaderProgram(nullptr) {}

FontLine2d::~FontLine2d() = default;

bool FontLine2d::Init(string filename, shared_ptr<Shader> _sh) {
    return character.Init(std::move(filename), std::move(_sh));
}

void FontLine2d::SetAspectRatio(int w, int h) {
    character.SetAspectRatio(w, h);
    aratio = static_cast<float>(h) / static_cast<float>(w);
}

void FontLine2d::SetText(string _text) { text = std::move(_text); }

// void FontLine2d::Render(string text, float startX, float startY,float size )
void FontLine2d::Render(const Camera& cam) {
    float dx = 0.0F;
    prevChar = 0;
    // float dy=0.0f;
    Vector2f temp;
    spaceWidth = character.GetSpaceWidth() /
                 (static_cast<float>(character.GetFontHeight()));
    for (unsigned int i = 0; i < text.length(); ++i) {
        // look up info about this character
        uint32_t const code = ((static_cast<uint32_t>(prevChar)) << 16) |
                              (static_cast<uint32_t>(text.at(i)));
        float kerning = 0;
        try {
            kerning = character.kerningInfo.at(code);
        } catch (std::out_of_range) {
            kerning = 0;
        }
        dx += kerning;

        // if we found a space character, draw it
        if (static_cast<unsigned int>(text.at(i)) ==
            static_cast<unsigned int>(' ')) {
            dx += position[2] * spaceWidth;
        }

        // actual drawing
        character.SetPosition(position[0] + dx, position[1], position[2]);
        character.SetCharacter(text.at(i));
        temp = character.GetLastCharacterLength();
        character.Render(cam);

        // if the character exists, draw it
        if (temp.x > 0.0F) {
            // space and separator between characters
            dx += temp.x + (position[2] * spaceWidth / 4.0F);
        } else {
            continue;  // no character - do not write anything
        }
        // remember the previous character
        prevChar = static_cast<unsigned int>(text.at(i));
    }
}

Font2d::Font2d() : aratio(1) { color = Vector4f(1.0F, 1.0F, 1.0F, 1.0F); }

Font2d::~Font2d() = default;

float Font2d::GetHeight(unsigned int c) const {
    FontCharacter const temp = fontInfo.at(c);
    float const realHeight =
        static_cast<float>(temp.height) / static_cast<float>(imageHeight);
    float const dx = 1.0F;
    return (2 * dx) * realHeight * ky;
}

float Font2d::GetWidth(unsigned int c) const {
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

int Font2d::GetFontHeight() const { return fontHeight; }

bool Font2d::Init(string _filename, shared_ptr<Shader> _sh) {
    shaderProgram = _sh;
    filename = _filename;
    string imgFilename;
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
    std::array<unsigned int, 4> indicies = {0, 1, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies.data(),
                 GL_STATIC_DRAW);

    /*===========Filling font information============*/
    fstream fin;
    fin.open(filename.c_str(), ios::in);
    // temporary variable for reading
    string in_s;
    bool kerning = false;
    bool data = false;
    while (1) {
        getline(fin, in_s);
        if (!fin.eof()) {
            // read data
            if (data) {
                stringstream sstr;
                unsigned int t1[6];
                int t2[2];
                sstr << in_s;
                unsigned int code = 0;
                sstr >> code;
                sstr >> t1[0] >> t1[1] >> t1[2] >> t1[3] >> t2[0] >> t2[1] >>
                    t1[4] >> t1[5];
                FontCharacter const temp2 = FontCharacter(
                    t1[0], t1[1], t1[2], t1[3], t2[0], t2[1], t1[4], t1[5]);
                fontInfo.insert(pair<unsigned int, FontCharacter>(code, temp2));
            }
            if (kerning) {
                stringstream sstr;
                sstr << in_s;
                uint16_t code1 = 0;
                sstr >> code1;
                uint16_t code2 = 0;
                float f1 = NAN;
                sstr >> code2 >> f1;
                uint32_t const code = (static_cast<uint32_t>(code1) << 16) |
                                      (static_cast<uint32_t>(code2));
                kerningInfo.insert(pair<uint32_t, float>(code, f1));
            }
            // find texture name
            int t = in_s.find("textures: ");
            string temp("textures: ");
            // found texture name
            if (t == 0) {
                imgFilename = string("fonts/") + string(in_s, temp.length());
                std::print("\nFont image is {}", imgFilename);
            }

            // find font name
            t = in_s.find("px");
            if (t >= 0) {
                int const t2 = in_s.find(' ');
                fontName = string(in_s, 0, t2);
                stringstream sstr;
                temp = string(in_s, t2, t - t2);
                sstr << temp;
                sstr >> fontHeight;
                // sscanf(in_s.c_str(),"%s %dpx",fontName,fontHeight);
                // flag that data is now being read
                data = true;
                std::print("\nFont name is {}", fontName);
                std::print("\nFont height is {}", fontHeight);
            }
            // find information about upcoming kerning
            t = in_s.find("kerning pairs:");
            if (t >= 0) {
                // stopped reading data
                data = false;
                // started reading kerning pairs
                kerning = true;
            }

        } else {
            {
                break;
            }
        }
        in_s.clear();
    }
    /*================================================*/

    // create texture
    stbi_set_flip_vertically_on_load(1);

    int width;
    int height;
    int nrChannels;
    // Load PNG. Pass 4 to have RGBA
    unsigned char* image_data =
        stbi_load(imgFilename.c_str(), &width, &height, &nrChannels, 4);

    if (image_data != nullptr) {
        // SOIL_FLAG_NTSC_SAFE_RGB -> Compress RGB values to the range [16, 235]
        // while leaving alpha channel
        for (int i = 0; i < width * height * 4; ++i) {
            if (i % 4 != 3) {  // Compress R, G, B. Alpha channel (every 4th
                               // byte) is left unchanged
                image_data[i] = 16 + (image_data[i] * (235 - 16) / 255);
            }
        }

        // SOIL_CREATE_NEW_ID -> Create a new texture ID and bind it
        glGenTextures(1, &texBufferID);
        glBindTexture(GL_TEXTURE_2D, texBufferID);

        // Setting texture filtering parameters for mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Asking videocard to compress the txture
        GLint const internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

        // Send pixels to Opengls
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image_data);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // free data
        stbi_image_free(image_data);
    } else {
        std::cerr << "[STB Error] Could not load file : " << imgFilename
                  << " | Reason: " << stbi_failure_reason() << '\n';
    }

    // activate texture unit 0
    glActiveTexture(GL_TEXTURE0);
    // bind texture to active texture unit
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    texSamplerID = shaderProgram->GetUniformLocation("texSampler");

    // Get image dimensions
    FILE* imageFile = fopen(imgFilename.c_str(), "rb");
    char buffer2[4];

    // Load header
    fread(&buffer2, 1, 4, imageFile);
    // printf("\nBuffer 0 %x",buffer);
    // printf("\nBuffer 0 %x %x %x
    // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    fread(&buffer2, 1, 4, imageFile);
    // printf("\nBuffer 1 %x %x %x
    // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    fread(&buffer2, 1, 4, imageFile);
    // printf("\nIHDR chank");
    // printf("\nBuffer 2 %x %x %x
    // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    fread(&buffer2, 1, 4, imageFile);
    // printf("\nBuffer 3 %x %x %x
    // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    fread(&buffer2, 1, 4, imageFile);
    // printf("\nBuffer 4 %x %x %x
    // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);

    // Calculate width
    imageWidth = (static_cast<uint32_t>(buffer2[3]) << 0) |
                 (static_cast<uint32_t>(buffer2[2]) << 8) |
                 (static_cast<uint32_t>(buffer2[1]) << 16) |
                 (static_cast<uint32_t>(buffer2[0]) << 24);
    std::print("\n width={}", imageWidth);

    // Calculate height
    fread(&buffer2, 1, 4, imageFile);
    // printf("\nBuffer 5 %x %x %x
    // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    imageHeight = (static_cast<uint32_t>(buffer2[3]) << 0) |
                  (static_cast<uint32_t>(buffer2[2]) << 8) |
                  (static_cast<uint32_t>(buffer2[1]) << 16) |
                  (static_cast<uint32_t>(buffer2[0]) << 24);
    std::print("\n height={}", imageHeight);

    // Convert pixel to relative coordinates
    pkx = 1.0F / static_cast<float>(imageWidth);
    pky = 1.0F / static_cast<float>(imageHeight);

    fclose(imageFile);

    return true;
}

void Font2d::SetAspectRatio(int _width, int _height) {
    aratio = static_cast<float>(_height) / static_cast<float>(_width);
    kx = static_cast<float>(imageWidth) / static_cast<float>(_width);
    ky = static_cast<float>(imageHeight) / static_cast<float>(_height);
}

void Font2d::SetAspectRatio(float f) { aratio = f; }

void Font2d::SetCharacter(unsigned int c) {
    character = c;

    // this is a mess, setting the Y coordinate
    position[2] = position[2] / static_cast<float>(fontHeight);
    temp = FontCharacter(0, 0, 0, 0, 0, 0, 0, 0);
    try {
        temp = fontInfo.at(character);
    } catch (const std::out_of_range& oor) {
        // printf("\n char is out of range");
        characterLength = Vector2f(-1.0F, -1.0F);
        return;
    }
    realWidth = static_cast<float>(temp.width) / static_cast<float>(imageWidth);
    realHeight =
        static_cast<float>(temp.height) / static_cast<float>(imageHeight);
    dx = 1.0F;
    xOffset = position[2] * (2 * dx) * kx * static_cast<float>(temp.xOffset) /
              static_cast<float>(imageWidth);
    yOffset = position[2] * (-2 * dx) * ky * static_cast<float>(temp.yOffset) /
              static_cast<float>(imageHeight);

    characterLength = Vector2f((2 * dx) * realWidth * kx * position[2],
                               (2 * dx) * realHeight * ky);
}

Vector2f Font2d::GetLastCharacterLength() {
    // return Vector2f((2*dx)*realWidth*kx*size,(2*dx)*realHeight*ky);
    return characterLength;
}

// Vector2f Font2d::Render(unsigned int c,float px,float py,float size)
void Font2d::Render(const Camera& cam) {
    SetAspectRatio(cam.GetWidth(), cam.GetHeight());
    shaderProgram->Use();
    /*position[2]=position[2]/(float)fontHeight;
    FontCharacter temp(0,0,0,0,0,0,0,0);
    try
    {
        temp=fontInfo.at(character);
    }
    catch(const std::out_of_range& oor)
    {
        //printf("\n char is out of range");
        return;// Vector2f(-1.0f,-1.0f);
    }
    float realWidth=(float)temp.width/(float)imageWidth;
    float realHeight=(float)temp.height/(float)imageHeight;
    float dx=1.0f;
    float xOffset=position[2]*(2*dx)*kx*(float)temp.xOffset/(float)imageWidth;
    float
    yOffset=position[2]*(-2*dx)*ky*(float)temp.yOffset/(float)imageHeight;*/

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
    float const u =
        static_cast<float>(temp.xpos) / static_cast<float>(imageWidth);
    float const v = 1.0F - (static_cast<float>(temp.ypos) /
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

Text2d::Text2d() : aratio(1) { color = Vector4f(1.0F, 1.0F, 1.0F, 1.0F); }

Text2d::~Text2d() = default;
void Text2d::Init(int width, int height, shared_ptr<Shader> _sh) {
    if (!_sh) {
        yourselfShader = true;
        aratio = static_cast<float>(height) / static_cast<float>(width);
        char const* vertexShaderSorceCode = ReadFile("shaders/text2d.vsh");
        char const* fragmentShaderSourceCode = ReadFile("shaders/text2d.fsh");
        shaderProgram = make_shared<Shader>();
        shaderProgram->AddShader(vertexShaderSorceCode, VertexShader);
        shaderProgram->AddShader(fragmentShaderSourceCode, FragmnetShader);
        shaderProgram->Init();
    } else {
        shaderProgram = _sh;
        yourselfShader = false;
    }
    std::array<unsigned int, 4> indicies = {0, 1, 2, 3};
    indicies[0] = 0;
    indicies[1] = 1;
    indicies[2] = 2;
    indicies[3] = 3;
    float const dx = 1;
    float vertices[] = {0,
                        0 - (dx * 2),
                        0,
                        0,
                        0 + (dx * aratio),
                        0 - (dx * 2),
                        (0 + (dx * aratio)),
                        0};
    float const dy = 0.0625;  //==1/16, высота одного квадрата
    float uvs[] = {
        0, 0 - (2 * dy), 0, 0, 0 + (dy / 2), 0 - (2 * dy), 0 + (dy / 2), 0};

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * 2, nullptr,
                 GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(vertices), &uvs);
    positionID = shaderProgram->GetAttribLocation("Position");
    uvID = shaderProgram->GetAttribLocation("UV");
    spositionID = shaderProgram->GetUniformLocation("s_Position");
    suvID = shaderProgram->GetUniformLocation("s_UV");
    colorID = shaderProgram->GetUniformLocation("textColor");

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies.data(),
                 GL_STATIC_DRAW);

    // create texture
    //  texBufferID = TextureCreateFromTGA("Textures/Anonymus Bold
    //  512x256.tga");

    stbi_set_flip_vertically_on_load(1);

    int nrChannels = 0;
    unsigned char* data = stbi_load("Textures/Anonymus 4096x2048.tga_sdf.png",
                                    &width, &height, &nrChannels, 4);

    if (data != nullptr) {
        // Compress 0-255 to "safe" 16-235
        for (int i = 0; i < width * height * 4; ++i) {
            if (i % 4 != 3) {  // Edit only RGB. Don't touch Alpha
                data[i] = 16 + (data[i] * (235 - 16) / 255);
            }
        }

        // Generate new textutre id
        glGenTextures(1, &texBufferID);
        glBindTexture(GL_TEXTURE_2D, texBufferID);

        // setup mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // compress to DXT5 on the fly
        GLint const internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

        // send pixels to VRAM
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);

        // genearete mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // free RAM
        stbi_image_free(data);
    } else {
        std::cerr << "[STB Error] Could not load: Textures/Anonymus "
                     "4096x2048.tga_sdf.png | Причина: "
                  << stbi_failure_reason() << '\n';
    }

    // make texture unit 0 active
    glActiveTexture(GL_TEXTURE0);
    // assign texture to active texture unit
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    texSamplerID = shaderProgram->GetUniformLocation("texSampler");
}

void Text2d::SetAspectRatio(int width, int height) {
    aratio = static_cast<float>(height) / static_cast<float>(width);
    float const dx = 1.0F;
    float vertices[] = {0,
                        0 - (dx * 2),
                        0,
                        0,
                        0 + (dx * aratio),
                        0 - (dx * 2),
                        (0 + (dx * aratio)),
                        0};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
}

void Text2d::SetAspectRatio(float f) {
    aratio = f;
    float const dx = 1.0F;
    float vertices[] = {0,
                        0 - (dx * 2),
                        0,
                        0,
                        0 + (dx * aratio),
                        0 - (dx * 2),
                        (0 + (dx * aratio)),
                        0};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
}

void Text2d::Init(shared_ptr<Shader> shader, GLuint textureID, GLuint texBuf) {
    // shaderProgramID=shader;
    shaderProgram = std::move(shader);
    yourselfShader = false;

    std::array<unsigned int, 4> indicies{};
    indicies[0] = 0;
    indicies[1] = 1;
    indicies[2] = 2;
    indicies[3] = 1;  // indicies[4]=2;indicies[5]=3;
    float vertices[] = {0, 0 - 1, 0, 0, 0 + 1, 0 - 1, (0 + 1), 0};
    float const dy = 0.0625;  // 1/16, высота одного квадрата
    float uvs[] = {0, 0 - dy, 0, 0, 0 + dy, 0 - dy, 0 + dy, 0};

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * 2, nullptr,
                 GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(vertices), &uvs);
    positionID = shaderProgram->GetAttribLocation("Position");
    uvID = shaderProgram->GetAttribLocation("UV");

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies.data(),
                 GL_STATIC_DRAW);

    // create texture
    texBufferID = texBuf;
    texSamplerID = textureID;
}

void Text2d::SetCharacter(unsigned int c) { character = c; }

void Text2d::Render(const Camera& /*cam*/) {
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float const dx = position[2] * 2;
    float const dy = 0.0625;  // 1/16, the height of one square
    // Compute index into the font texture atlas
    const unsigned int CYRILLIC_START = 0x0410;  // 'А'
    unsigned int num =
        character -
        32;  // 32 characters were missed while generating the texture.
    if (character > CYRILLIC_START) {
        num = character + 224;
    }
    // 16 строк и 16 столбцов
    float const x = (static_cast<double>(num % 32)) / 32;
    float const y = 1 - (static_cast<float>((unsigned)num / 32) / 8);
    positionID = shaderProgram->GetAttribLocation("Position");
    uvID = shaderProgram->GetAttribLocation("UV");
    positionID = shaderProgram->GetUniformLocation("s_Position");
    suvID = shaderProgram->GetUniformLocation("s_UV");
    sizeID = shaderProgram->GetUniformLocation("size");
    colorID = shaderProgram->GetUniformLocation("textColor");
    // vector position in space
    glUniform2f(spositionID, position[0], position[1]);
    // UV offset vector
    glUniform2f(suvID, x, y);
    glUniform1f(sizeID, dx);

    glVertexAttribPointer(positionID, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(uvID, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(float) * 8));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glActiveTexture(GL_TEXTURE0);
    // assign texture to active texture unit
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    glUniform1i(texSamplerID,
                0);  // tell shader to use texture unit 0 as the sampler
    glUniform4f(colorID, color.r, color.g, color.b, color.a);
    glEnableVertexAttribArray(positionID);
    glEnableVertexAttribArray(uvID);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(positionID);
    glDisableVertexAttribArray(uvID);
}

TextLine2d::TextLine2d() : aratio(1) {}

TextLine2d::~TextLine2d() = default;

void TextLine2d::SetAspectRatio(int width, int height) {
    aratio = static_cast<float>(height) / static_cast<float>(width);
    if (symbol != nullptr) {
        symbol->SetAspectRatio(aratio);
    }
}

void TextLine2d::Init(int width, int height, shared_ptr<Shader> _sh) {
    symbol = std::make_unique<Text2d>();
    pixelSize =
        static_cast<float>(512) /
        (static_cast<float>(width) *
         static_cast<float>(16));  // 512 размер текстуры, 16 квадратов в ней
    // symbol->SetAspectRatio(aratio);
    symbol->Init(width, height, std::move(_sh));
}

void TextLine2d::SetText(string _text) { text = std::move(_text); }

void TextLine2d::Render(const Camera& cam) {
    float delta = 0;
    for (unsigned int i = 0; i < strlen(text.c_str());
         i++, delta += position[2] * pixelSize * aratio * 2.0 /
                       32.0)  // 1.9 подобрано экспериментально
    {
        // symbol->Render((unsigned
        // char)input[i],x+delta,y,size*pixelSize/32.0);
        symbol->SetCharacter(static_cast<unsigned char>(text[i]));
        symbol->SetPosition(position[0] + delta, position[1],
                            position[2] * pixelSize / 32.0);
        symbol->Render(cam);
    }
}
