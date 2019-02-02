#include "Text_2D.h"
#include <fstream>
#include <sstream>
#include <stdexcept>  // std::out_of_range

FontLine2d::FontLine2d() { shaderProgram = nullptr; }

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
void FontLine2d::Render(Camera* cam) {
    float dx = 0.0f;
    prevChar = 0;
    // float dy=0.0f;
    Vector2f temp;
    spaceWidth = character.GetSpaceWidth() / ((float)character.GetFontHeight());
    for (unsigned int i = 0; i < text.length(); ++i) {
        //���� ���� �� ���� �������
        uint32_t code = (((uint32_t)prevChar) << 16) | ((uint32_t)text.at(i));
        float kerning = 0;
        try {
            kerning = character.kerningInfo.at(code);
        } catch (std::out_of_range) {
            kerning = 0;
        }
        dx += kerning;

        //���� ����� ������ ������� ������ ���
        if ((unsigned int)text.at(i) == (unsigned int)' ') {
            dx += position[2] * spaceWidth;
        }

        //������ �������
        character.SetPosition(position[0] + dx, position[1], position[2]);
        character.SetCharacter(text.at(i));
        temp = character.GetLastCharacterLength();
        character.Render(cam);

        //���� ������ ����, ������ ���
        if (temp.x > 0.0f)
            //������ � ����������� ����� �������
            dx += temp.x + position[2] * spaceWidth / 4.0f;
        else
            continue;  //��� ������� - ������ �� �����
        //���������� ���������� ������
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
    //����������� ��������� ������ �� �������
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //� ��������� ������
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    //�������� �������� ��������� ����������
    sverticesID = shaderProgram->GetAttribLocation("Position");
    uvID = shaderProgram->GetAttribLocation("UV");
    spositionID = shaderProgram->GetUniformLocation("s_Position");
    suvID = shaderProgram->GetUniformLocation("s_UV");
    colorID = shaderProgram->GetUniformLocation("textColor");

    //�� � ���������� ��������
    indicies = new unsigned int[4];
    indicies[0] = 0;
    indicies[1] = 1;
    indicies[2] = 2;
    indicies[3] = 3;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 4, indicies,
                 GL_STATIC_DRAW);

    /*===========��������� ���������� � ������============*/
    fstream fin;
    fin.open(filename.c_str(), ios::in);
    //��������� ���������� ��� ������
    string in_s;
    bool kerning = false;
    bool data = false;
    while (1) {
        getline(fin, in_s);
        if (!fin.eof()) {
            //������ ������
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
            //���� �������� ��������
            int t = in_s.find("textures: ");
            string temp("textures: ");
            //����� �������� ��������
            if (t == 0) {
                imgFilename = string("Fonts/") + string(in_s, temp.length());
                printf("\nFont image is %s", imgFilename.c_str());
            }

            //���� �������� ������
            t = in_s.find("px");
            if (t >= 0) {
                int t2 = in_s.find(" ");
                fontName = string(in_s, 0, t2);
                stringstream sstr;
                temp = string(in_s, t2, t - t2);
                sstr << temp;
                sstr >> fontHeight;
                // sscanf(in_s.c_str(),"%s %dpx",fontName,fontHeight);
                //���� � ���, ��� ������ ����� �������� ������
                data = true;
                printf("\nFont name is %s", fontName.c_str());
                printf("\nFont height is %d", fontHeight);
            }
            //���� ���������� � ���, ��� ������ ����� �������
            t = in_s.find("kerning pairs:");
            if (t >= 0) {
                //��������� ������ ������
                data = false;
                //������ ������ � ����� ��������
                kerning = true;
            }

        } else
            break;
        in_s.clear();
    }
    /*================================================*/

    //������ ��������
    texBufferID = SOIL_load_OGL_texture(
        (char*)imgFilename.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB |
            SOIL_FLAG_COMPRESS_TO_DXT);
    // ������ �������� ���������� ���� 0
    glActiveTexture(GL_TEXTURE0);
    // ��������� �������� �� �������� ���������� ����
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    texSamplerID = shaderProgram->GetUniformLocation("texSampler");

    //�������� ������� �����������
    FILE* imageFile = fopen(imgFilename.c_str(), "rb");
    char buffer2[4];

    //��������� ������������
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

    //������� ������ ������
    imageWidth = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
                 ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] << 24);
    printf("\n width=%d", imageWidth);

    //������� ������
    fread(&buffer2, 1, 4, imageFile);
    // printf("\nBuffer 5 %x %x %x
    // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    imageHeight = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
                  ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] << 24);
    printf("\n height=%d", imageHeight);

    //��������� ������� � ������������� ����������
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

    //�� ������, ��������� ���������� Y
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
void Font2d::Render(Camera* cam) {
    SetAspectRatio(cam->GetWidth(), cam->GetHeight());
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
    //������� ��������
    // px+=((float)temp.xOffset/(float)imageWidth)*kx*1.0f*size;
    // py-=((float)temp.yOffset/(float)imageHeight)*ky*1.0f*size;

    //������ ��������� � ������������
    glUniform2f(spositionID, position[0] + xOffset, position[1] + yOffset);
    //������ �������� UV
    glUniform2f(suvID, u, v);
    //������
    glUniform1f(sizeID, position[2]);

    glVertexAttribPointer(sverticesID, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(uvID, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(float) * 8));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glActiveTexture(GL_TEXTURE0);
    // ��������� �������� �� �������� ���������� ����
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    glUniform1i(texSamplerID,
                0);  //������� �������, ����� ����������� � �������� �������� 0
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
    if (_sh == nullptr) {
        yourselfShader = true;
        aratio = (float)height / (float)width;
        char* vertexShaderSorceCode = ReadFile("shaders/text2d.vsh");
        char* fragmentShaderSourceCode = ReadFile("shaders/text2d.fsh");
        shaderProgram = make_shared<Shader>();
        shaderProgram->Add(Shader::Type::kVertex,
                           std::string(vertexShaderSorceCode));
        shaderProgram->Add(Shader::Type::kFragmnet,
                           std::string(fragmentShaderSourceCode));
    } else {
        shaderProgram = _sh;
        yourselfShader = false;
    }
    indicies = new unsigned int[6];
    indicies[0] = 0;
    indicies[1] = 1;
    indicies[2] = 2;
    indicies[3] = 3;
    float dx = 1;
    float vertices[] = {0,          0 - dx * 2,        0, 0, 0 + dx * aratio,
                        0 - dx * 2, (0 + dx * aratio), 0};
    float dy = 0.0625;  //==1/16, ������ ������ ��������
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 4, indicies,
                 GL_STATIC_DRAW);

    //������ ��������
    // texBufferID = TextureCreateFromTGA("textures/Anonymus Bold 512x256.tga");
    texBufferID = SOIL_load_OGL_texture(
        "textures/Anonymus 4096x2048.tga_sdf.png", SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB |
            SOIL_FLAG_COMPRESS_TO_DXT);
    // ������ �������� ���������� ���� 0
    glActiveTexture(GL_TEXTURE0);
    // ��������� �������� �� �������� ���������� ����
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
    indicies = new unsigned int[6];
    indicies[0] = 0;
    indicies[1] = 1;
    indicies[2] = 2;
    indicies[3] = 1;  // indicies[4]=2;indicies[5]=3;
    float vertices[] = {0, 0 - 1, 0, 0, 0 + 1, 0 - 1, (0 + 1), 0};
    float dy = 0.0625;  // 1/16, ������ ������ ��������
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 4, indicies,
                 GL_STATIC_DRAW);

    //������ ��������
    texBufferID = texBuf;
    texSamplerID = textureID;
}

void Text2d::SetCharacter(unsigned int c) { character = c; }

void Text2d::Render(Camera* cam) {
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float dx = position[2] * 2;
    // float dy=0.0625; //1/16, ������ ������ ��������
    unsigned int num =
        character - 32;  // 32 ������� ��������� ��� ������������� ��������
    if (character > (unsigned char)'�') num = character + 224;
    // 16 ����� � 16 ��������
    float x = ((double)(num % 32)) / 32;
    float y = 1 - ((float)((unsigned)num / 32) / 8);
    positionID = shaderProgram->GetAttribLocation("Position");
    uvID = shaderProgram->GetAttribLocation("UV");
    positionID = shaderProgram->GetUniformLocation("s_Position");
    suvID = shaderProgram->GetUniformLocation("s_UV");
    sizeID = shaderProgram->GetUniformLocation("size");
    colorID = shaderProgram->GetUniformLocation("textColor");
    //������ ��������� � ������������
    glUniform2f(spositionID, position[0], position[1]);
    //������ �������� UV
    glUniform2f(suvID, x, y);
    glUniform1f(sizeID, dx);

    glVertexAttribPointer(positionID, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(uvID, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(float) * 8));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glActiveTexture(GL_TEXTURE0);
    // ��������� �������� �� �������� ���������� ����
    glBindTexture(GL_TEXTURE_2D, texBufferID);
    glUniform1i(texSamplerID,
                0);  //������� �������, ����� ����������� � �������� �������� 0
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
    symbol = new Text2d;
    pixelSize =
        (float)(512) /
        ((float)width * (float)16);  // 512 ������ ��������, 16 ��������� � ���
    // symbol->SetAspectRatio(aratio);
    symbol->Init(width, height, _sh);
}

void TextLine2d::SetText(string _text) { text = _text; }

void TextLine2d::Render(Camera* cam) {
    float delta = 0;
    for (unsigned int i = 0; i < strlen(text.c_str());
         i++, delta += position[2] * pixelSize * aratio * 2.0 /
                       32.0)  // 1.9 ��������� ����������������
    {
        // symbol->Render((unsigned
        // char)input[i],x+delta,y,size*pixelSize/32.0);
        symbol->SetCharacter((unsigned char)text[i]);
        symbol->SetPosition(position[0] + delta, position[1],
                            position[2] * pixelSize / 32.0);
        symbol->Render(cam);
    }
}
