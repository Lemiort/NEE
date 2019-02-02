#include "Font2d.h"

Character2d::Character2d() {}

Character2d::~Character2d() {}

bool Character2d::Init(shared_ptr<Material> _mat, string _fileName) {
    mesh.Init(_mat, "models/quad2x2front.ho3d");
    this->mat = _mat;

    // TODO ������� �������� ������
    fileName = _fileName;

    //��������� ���� ������ �� ������
    fstream fin;
    fin.open(fileName.c_str(), ios::in);
    //��������� ���������� ��� ������
    string inStr;

    //��������� ���������� ��� �������� ��������
    string imgFilename;

    //���� ����, ��� ����� ������ � ��������
    bool kerning = false;
    //���� ����, ��� ����� ������ � ��������
    bool data = false;
    while (1) {
        //������ ���������
        getline(fin, inStr);
        //���� �� ��������� ����� �����
        if (!fin.eof()) {
            //������ ������
            if (data) {
                //����� ��� �����������
                stringstream sstr;
                //???
                unsigned int t1[6];
                //???
                int t2[2];
                sstr << inStr;
                unsigned int code;
                sstr >> code;
                sstr >> t1[0] >> t1[1] >> t1[2] >> t1[3] >> t2[0] >> t2[1] >>
                    t1[4] >> t1[5];
                //�������������� ������ � ����� �����
                FontCharacter temp2 = FontCharacter(t1[0], t1[1], t1[2], t1[3],
                                                    t2[0], t2[1], t1[4], t1[5]);
                //��������� ������ � ����� � ������
                fontInfo.insert(pair<unsigned int, FontCharacter>(code, temp2));
            }
            //��������� ���� � ��������
            if (kerning) {
                stringstream sstr;
                sstr << inStr;
                uint16_t code1;
                sstr >> code1;
                uint16_t code2;
                float f1;
                sstr >> code2 >> f1;
                uint32_t code = ((uint32_t)code1 << 16) | ((uint32_t)code2);
                //��������� ������ � ��������
                kerningInfo.insert(pair<uint32_t, float>(code, f1));
            }
            //���� �������� ��������
            int t = inStr.find("textures: ");
            string temp("textures: ");
            //����� �������� ��������
            if (t == 0) {
                imgFilename = string("Fonts/") + string(inStr, temp.length());
                printf("\nFont image is %s", imgFilename.c_str());
            }

            //���� �������� ������
            t = inStr.find("px");
            if (t >= 0) {
                int t2 = inStr.find(" ");
                fontName = string(inStr, 0, t2);
                stringstream sstr;
                temp = string(inStr, t2, t - t2);
                sstr << temp;
                sstr >> fontHeight;

                //���� � ���, ��� ������ ����� �������� ������
                data = true;
                printf("\nFont name is %s", fontName.c_str());
                printf("\nFont height is %d", fontHeight);
            }
            //���� ���������� � ���, ��� ������ ����� �������
            t = inStr.find("kerning pairs:");
            if (t >= 0) {
                //��������� ������ ������
                data = false;
                //������ ������ � ����� ��������
                kerning = true;
            }

        } else
            break;
        inStr.clear();
    }  //��������� ������ � ������

    GLuint texBufferID;
    //������ ��������
    texBufferID = SOIL_load_OGL_texture(
        (char*)imgFilename.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB |
            SOIL_FLAG_COMPRESS_TO_DXT);
    //��������� � ��������
    shared_ptr<Texture2D> temp(new Texture2D(texBufferID));
    _mat->SetColorTexture(temp);

    //�������� ������� �����������
    FILE* imageFile = fopen(imgFilename.c_str(), "rb");
    char buffer2[4];

    //��������� ������������
    // TODO ����������� � ���� �����
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

float Character2d::GetAspectRatio() { return aratio; }

void Character2d::SetAspectRatio(int _width, int _height) {
    aratio = (float)_height / (float)_width;
    kx = (float)imageWidth / (float)_width;
    ky = (float)imageHeight / (float)_height;
}

void Character2d::Render(Camera* cam) {
    SetAspectRatio(cam->GetWidth(), cam->GetHeight());
    this->mat->Use();

    // TODO ����: ������������� ���������� ���������� � �������
    /*float vertices[]={0.0f,(-2*dx)*realHeight*ky,
                                       0.0f,0.0f,
                                       (2*dx)*realWidth*kx,(-2*dx)*realHeight*ky,
                                       (2*dx)*realWidth*kx,0.0f};

   float uvs[]={0.0f,-realHeight,
                     0.0f,0.0f,
                     realWidth,-realHeight,
                                         realWidth,0.0f};*/

    // mesh->SetScale()
}

float Character2d::GetHeight(unsigned int c) {
    FontCharacter temp = fontInfo.at(c);
    float realHeight = (float)temp.height / (float)imageHeight;
    float dx = 1.0f;
    return (2 * dx) * realHeight * ky;
}

float Character2d::GetWidth(unsigned int c) {
    FontCharacter temp = fontInfo.at(c);
    float realWidth = (float)temp.width / (float)imageWidth;
    float dx = 1.0f;
    return (2 * dx) * realWidth * kx;
}

float Character2d::GetSpaceWidth() {
    FontCharacter temp = fontInfo.at(34);
    float realWidth = (float)temp.origW / (float)imageWidth;
    float dx = 1.0f;
    return (2 * dx) * realWidth * kx;
}

int Character2d::GetFontHeight() { return fontHeight; }

void Character2d::SetCharacter(unsigned int c) {
    currentCharacter = c;

    // TODO ���������������???
    position[2] = position[2] / (float)fontHeight;
    temp = FontCharacter(0, 0, 0, 0, 0, 0, 0, 0);
    //���� ���� � ������� �������
    try {
        temp = fontInfo.at(currentCharacter);
    } catch (const std::out_of_range& oor) {
        // printf("\n char is out of range");
        characterLength = glm::vec2(-1.0f, -1.0f);
        return;
    }
    //������ � ������ � uv-�����������
    realWidth = (float)temp.width / (float)imageWidth;
    realHeight = (float)temp.height / (float)imageHeight;
    dx = 1.0f;
    //������� �� ��������
    xOffset =
        position[2] * (2 * dx) * kx * (float)temp.xOffset / (float)imageWidth;
    yOffset =
        position[2] * (-2 * dx) * ky * (float)temp.yOffset / (float)imageHeight;

    // TODO ������� ����� ������� � ���������� �����������??
    characterLength = glm::vec2((2 * dx) * realWidth * kx * position[2],
                                (2 * dx) * realHeight * ky);
}

glm::vec2 Character2d::GetLastCharacterLength() {
    // return glm::vec2((2*dx)*realWidth*kx*size,(2*dx)*realHeight*ky);
    return characterLength;
}
