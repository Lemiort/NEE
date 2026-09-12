#include "Text2D.hpp"

#include <array>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>  // std::out_of_range]

#include "stb_image.h"

// The shader program is managed via a shared_ptr. Initialize it to nullptr.
FontLine2d::FontLine2d() : shaderProgram(nullptr) {}

FontLine2d::~FontLine2d() {}

bool FontLine2d::Init(string filename, shared_ptr<Shader> _sh) {
    return character.Init(filename, _sh);
}

void FontLine2d::SetAspectRatio(int w, int h) {
    character.SetAspectRatio(w, h);
    aratio = (float)h / (float)w;
}

void FontLine2d::SetText(string _text) { text = _text; }

// void FontLine2d::Render(string text, float startX, float startY,float size )
void FontLine2d::Render(const Camera& cam) {
    float dx = 0.0f;
    prevChar = 0;
    // float dy=0.0f;
    Vector2f temp;
    spaceWidth = character.GetSpaceWidth() / ((float)character.GetFontHeight());
    for (unsigned int i = 0; i < text.length(); ++i) {
        // look up info about this character
        uint32_t code = (((uint32_t)prevChar) << 16) | ((uint32_t)text.at(i));
        float kerning = 0;
        try {
            kerning = character.kerningInfo.at(code);
        } catch (std::out_of_range) {
            kerning = 0;
        }
        dx += kerning;

        // if we found a space character, draw it
        if ((unsigned int)text.at(i) == (unsigned int)' ') {
            dx += position[2] * spaceWidth;
        }

        // actual drawing
        character.SetPosition(position[0] + dx, position[1], position[2]);
        character.SetCharacter(text.at(i));
        temp = character.GetLastCharacterLength();
        character.Render(cam);

        // if the character exists, draw it
        if (temp.x > 0.0f)
            // space and separator between characters
            dx += temp.x + position[2] * spaceWidth / 4.0f;
        else
            continue;  // no character - do not write anything
        // remember the previous character
        prevChar = (unsigned int)text.at(i);
    }
}

Font2d::Font2d() {
    color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
    aratio = 1;
}

Font2d::~Font2d() {}

float Font2d::GetHeight(unsigned int c) {
    FontCharacter temp = fontInfo.at(c);
    float realHeight = (float)temp.height / (float)imageHeight;
    float dx = 1.0f;
    return (2 * dx) * realHeight * ky;
}

float Font2d::GetWidth(unsigned int c) {
    FontCharacter temp = fontInfo.at(c);
    float realWidth = (float)temp.width / (float)imageWidth;
    float dx = 1.0f;
    return (2 * dx) * realWidth * kx;
}

float Font2d::GetSpaceWidth() {
    FontCharacter temp = fontInfo.at(34);
    float realWidth = (float)temp.origW / (float)imageWidth;
    float dx = 1.0f;
    return (2 * dx) * realWidth * kx;
}

int Font2d::GetFontHeight() { return fontHeight; }

bool Font2d::Init(string _filename, shared_ptr<Shader> _sh) {
    shaderProgram = _sh;
    filename = _filename;
    string fntFilemame = _filename;
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
                unsigned int code;
                sstr >> code;
                sstr >> t1[0] >> t1[1] >> t1[2] >> t1[3] >> t2[0] >> t2[1] >>
                    t1[4] >> t1[5];
                FontCharacter temp2 = FontCharacter(t1[0], t1[1], t1[2], t1[3],
                                                    t2[0], t2[1], t1[4], t1[5]);
                fontInfo.insert(pair<unsigned int, FontCharacter>(code, temp2));
            }
            if (kerning) {
                stringstream sstr;
                sstr << in_s;
                uint16_t code1;
                sstr >> code1;
                uint16_t code2;
                float f1;
                sstr >> code2 >> f1;
                uint32_t code = ((uint32_t)code1 << 16) | ((uint32_t)code2);
                kerningInfo.insert(pair<uint32_t, float>(code, f1));
            }
            // find texture name
            int t = in_s.find("textures: ");
            string temp("textures: ");
            // found texture name
            if (t == 0) {
                imgFilename = string("fonts/") + string(in_s, temp.length());
                printf("\nFont image is %s", imgFilename.c_str());
            }

            // find font name
            t = in_s.find("px");
            if (t >= 0) {
                int t2 = in_s.find(" ");
                fontName = string(in_s, 0, t2);
                stringstream sstr;
                temp = string(in_s, t2, t - t2);
                sstr << temp;
                sstr >> fontHeight;
                // sscanf(in_s.c_str(),"%s %dpx",fontName,fontHeight);
                // flag that data is now being read
                data = true;
                printf("\nFont name is %s", fontName.c_str());
                printf("\nFont height is %d", fontHeight);
            }
            // find information about upcoming kerning
            t = in_s.find("kerning pairs:");
            if (t >= 0) {
                // stopped reading data
                data = false;
                // started reading kerning pairs
                kerning = true;
            }

        } else
            break;
        in_s.clear();
    }
    /*================================================*/

    // create texture
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    // Load PNG. Pass 4 to have RGBA
    unsigned char* image_data =
        stbi_load(imgFilename.c_str(), &width, &height, &nrChannels, 4);

    if (image_data) {
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
        GLint internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

        // Send pixels to Opengls
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image_data);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // free data
        stbi_image_free(image_data);
    } else {
        std::cerr << "[STB Error] Could not load file : " << imgFilename
                  << " | Reason: " << stbi_failure_reason() << std::endl;
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
    imageWidth = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
                 ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] << 24);
    printf("\n width=%d", imageWidth);

    // Calculate height
    fread(&buffer2, 1, 4, imageFile);
    // printf("\nBuffer 5 %x %x %x
    // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    imageHeight = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
                  ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] << 24);
    printf("\n height=%d", imageHeight);

    // Convert pixel to relative coordinates
    pkx = 1.0f / (float)imageWidth;
    pky = 1.0f / (float)imageHeight;

    fclose(imageFile);

    return true;
}

void Font2d::SetAspectRatio(int _width, int _height) {
    aratio = (float)_height / (float)_width;
    kx = (float)imageWidth / (float)_width;
    ky = (float)imageHeight / (float)_height;
}

void Font2d::SetAspectRatio(float f) { aratio = f; }

void Font2d::SetCharacter(unsigned int c) {
    character = c;

    // this is a mess, setting the Y coordinate
    position[2] = position[2] / (float)fontHeight;
    temp = FontCharacter(0, 0, 0, 0, 0, 0, 0, 0);
    try {
        temp = fontInfo.at(character);
    } catch (const std::out_of_range& oor) {
        // printf("\n char is out of range");
        characterLength = Vector2f(-1.0f, -1.0f);
        return;
    }
    realWidth = (float)temp.width / (float)imageWidth;
    realHeight = (float)temp.height / (float)imageHeight;
    dx = 1.0f;
    xOffset =
        position[2] * (2 * dx) * kx * (float)temp.xOffset / (float)imageWidth;
    yOffset =
        position[2] * (-2 * dx) * ky * (float)temp.yOffset / (float)imageHeight;

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
    float u = (float)(temp.xpos) / (float)imageWidth;
    float v = 1.0f - (float)(temp.ypos) / (float)imageHeight;
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

Text2d::Text2d() {
    color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
    aratio = 1;
}

Text2d::~Text2d() {}
void Text2d::Init(int width, int height, shared_ptr<Shader> _sh) {
    if (!_sh) {
        yourselfShader = true;
        aratio = (float)height / (float)width;
        char* vertexShaderSorceCode = ReadFile("shaders/text2d.vsh");
        char* fragmentShaderSourceCode = ReadFile("shaders/text2d.fsh");
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
    float dx = 1;
    float vertices[] = {0,          0 - dx * 2,        0, 0, 0 + dx * aratio,
                        0 - dx * 2, (0 + dx * aratio), 0};
    float dy = 0.0625;  //==1/16, высота одного квадрата
    float uvs[] = {0, 0 - 2 * dy, 0, 0, 0 + dy / 2, 0 - 2 * dy, 0 + dy / 2, 0};

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

    stbi_set_flip_vertically_on_load(true);

    int nrChannels;
    unsigned char* data = stbi_load("Textures/Anonymus 4096x2048.tga_sdf.png",
                                    &width, &height, &nrChannels, 4);

    if (data) {
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
        GLint internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

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
                  << stbi_failure_reason() << std::endl;
    }

    // make texture unit 0 active
    glActiveTexture(GL_TEXTURE0);
    // assign texture to active texture unit
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    texSamplerID = shaderProgram->GetUniformLocation("texSampler");
}

void Text2d::SetAspectRatio(int width, int height) {
    aratio = (float)height / (float)width;
    float dx = 1.0f;
    float vertices[] = {0,          0 - dx * 2,        0, 0, 0 + dx * aratio,
                        0 - dx * 2, (0 + dx * aratio), 0};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
}

void Text2d::SetAspectRatio(float f) {
    aratio = f;
    float dx = 1.0f;
    float vertices[] = {0,          0 - dx * 2,        0, 0, 0 + dx * aratio,
                        0 - dx * 2, (0 + dx * aratio), 0};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
}

void Text2d::Init(shared_ptr<Shader> shader, GLuint textureID, GLuint texBuf) {
    // shaderProgramID=shader;
    shaderProgram = shader;
    yourselfShader = false;

    std::array<unsigned int, 4> indicies;
    indicies[0] = 0;
    indicies[1] = 1;
    indicies[2] = 2;
    indicies[3] = 1;  // indicies[4]=2;indicies[5]=3;
    float vertices[] = {0, 0 - 1, 0, 0, 0 + 1, 0 - 1, (0 + 1), 0};
    float dy = 0.0625;  // 1/16, высота одного квадрата
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

void Text2d::Render(const Camera& cam) {
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float dx = position[2] * 2;
    // float dy=0.0625; //1/16, высота одного квадрата
    unsigned int num =
        character - 32;  // 32 символа пропущено при генерировании текстуры
    if (character > (unsigned char)'А') num = character + 224;
    // 16 строк и 16 столбцов
    float x = ((double)(num % 32)) / 32;
    float y = 1 - ((float)((unsigned)num / 32) / 8);
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

TextLine2d::TextLine2d() { aratio = 1; }

TextLine2d::~TextLine2d() {}

void TextLine2d::SetAspectRatio(int width, int height) {
    aratio = (float)height / (float)width;
    if (symbol != NULL) symbol->SetAspectRatio(aratio);
}

void TextLine2d::Init(int width, int height, shared_ptr<Shader> _sh) {
    symbol = std::make_unique<Text2d>();
    pixelSize =
        (float)(512) /
        ((float)width * (float)16);  // 512 размер текстуры, 16 квадратов в ней
    // symbol->SetAspectRatio(aratio);
    symbol->Init(width, height, _sh);
}

void TextLine2d::SetText(string _text) { text = _text; }

void TextLine2d::Render(const Camera& cam) {
    float delta = 0;
    for (unsigned int i = 0; i < strlen(text.c_str());
         i++, delta += position[2] * pixelSize * aratio * 2.0 /
                       32.0)  // 1.9 подобрано экспериментально
    {
        // symbol->Render((unsigned
        // char)input[i],x+delta,y,size*pixelSize/32.0);
        symbol->SetCharacter((unsigned char)text[i]);
        symbol->SetPosition(position[0] + delta, position[1],
                            position[2] * pixelSize / 32.0);
        symbol->Render(cam);
    }
}
