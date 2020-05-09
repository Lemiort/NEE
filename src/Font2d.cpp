#include "Font2d.h"

#include <fstream>

Character2d::Character2d() {}

Character2d::~Character2d() {}

bool Character2d::Init(std::shared_ptr<Material> _mat, std::string _fileName) {
    mesh.Init(_mat, "models/quad2x2front.ho3d");
    this->mat = _mat;

    /**
     * @todo add font laoding
     */
    fileName = _fileName;

    std::ifstream fin;
    fin.open(fileName.c_str());
    std::string inStr;
    std::string imgFilename;

    bool kerning = false;
    bool data = false;
    while (1) {
        getline(fin, inStr);
        if (!fin.eof()) {
            if (data) {
                std::stringstream sstr;
                // ???
                unsigned int t1[6];
                // ???
                int t2[2];
                sstr << inStr;
                unsigned int code;
                sstr >> code;
                sstr >> t1[0] >> t1[1] >> t1[2] >> t1[3] >> t2[0] >> t2[1] >>
                    t1[4] >> t1[5];
                // fill character data
                FontCharacter temp2 = FontCharacter(t1[0], t1[1], t1[2], t1[3],
                                                    t2[0], t2[1], t1[4], t1[5]);
                // save character
                fontInfo.insert(
                    std::pair<unsigned int, FontCharacter>(code, temp2));
            }
            if (kerning) {
                std::stringstream sstr;
                sstr << inStr;
                uint16_t code1;
                sstr >> code1;
                uint16_t code2;
                float f1;
                sstr >> code2 >> f1;
                uint32_t code = ((uint32_t)code1 << 16) | ((uint32_t)code2);
                kerningInfo.insert(std::pair<uint32_t, float>(code, f1));
            }
            // find texture name
            int t = inStr.find("textures: ");
            std::string temp("textures: ");
            if (t == 0) {
                imgFilename =
                    std::string("fonts/") + std::string(inStr, temp.length());
                printf("\nFont image is %s", imgFilename.c_str());
            }

            // find font name
            t = inStr.find("px");
            if (t >= 0) {
                int t2 = inStr.find(" ");
                fontName = std::string(inStr, 0, t2);
                std::stringstream sstr;
                temp = std::string(inStr, t2, t - t2);
                sstr << temp;
                sstr >> fontHeight;

                data = true;
                printf("\nFont name is %s", fontName.c_str());
                printf("\nFont height is %d", fontHeight);
            }
            // kerning section
            t = inStr.find("kerning pairs:");
            if (t >= 0) {
                data = false;
                kerning = true;
            }
        } else {
            break;
        }
        inStr.clear();
    }

    GLuint texBufferID;
    texBufferID = SOIL_load_OGL_texture(
        imgFilename.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB |
            SOIL_FLAG_COMPRESS_TO_DXT);
    std::shared_ptr<Texture2D> temp(new Texture2D(texBufferID));
    _mat->SetColorTexture(temp);

    // read png header
    FILE* imageFile = fopen(imgFilename.c_str(), "rb");
    char buffer2[4];

    // load header
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

    // pixel->uv conversion
    pkx = 1.0f / static_cast<float>(imageWidth);
    pky = 1.0f / static_cast<float>(imageHeight);

    fclose(imageFile);

    return true;
}

float Character2d::GetAspectRatio() { return aspect_ratio; }

void Character2d::SetAspectRatio(int _width, int _height) {
    aspect_ratio = static_cast<float>(_height) / static_cast<float>(_width);
    kx = static_cast<float>(imageWidth) / static_cast<float>(_width);
    ky = static_cast<float>(imageHeight) / static_cast<float>(_height);
}

void Character2d::Render(const Camera& cam) {
    SetAspectRatio(cam.GetWidth(), cam.GetHeight());
    this->mat->Use();

    // @todo try to edit texture coordinates in shader
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
    float realHeight =
        static_cast<float>(temp.height) / static_cast<float>(imageHeight);
    float dx = 1.0f;
    return (2 * dx) * realHeight * ky;
}

float Character2d::GetWidth(unsigned int c) {
    FontCharacter temp = fontInfo.at(c);
    float realWidth =
        static_cast<float>(temp.width) / static_cast<float>(imageWidth);
    float dx = 1.0f;
    return (2 * dx) * realWidth * kx;
}

float Character2d::GetSpaceWidth() {
    FontCharacter temp = fontInfo.at(34);
    float realWidth =
        static_cast<float>(temp.origW) / static_cast<float>(imageWidth);
    float dx = 1.0f;
    return (2 * dx) * realWidth * kx;
}

int Character2d::GetFontHeight() { return fontHeight; }

void Character2d::SetCharacter(unsigned int c) {
    currentCharacter = c;

    // @todo scaling??
    position[2] = position[2] / static_cast<float>(fontHeight);
    temp = FontCharacter(0, 0, 0, 0, 0, 0, 0, 0);
    // find currecnt character info
    try {
        temp = fontInfo.at(currentCharacter);
    } catch (const std::out_of_range& oor) {
        // printf("\n char is out of range");
        characterLength = glm::vec2(-1.0f, -1.0f);
        return;
    }
    // width and height in uv
    realWidth = static_cast<float>(temp.width) / static_cast<float>(imageWidth);
    realHeight =
        static_cast<float>(temp.height) / static_cast<float>(imageHeight);
    dx = 1.0f;
    // positions in uv on texture
    xOffset = position[2] * (2 * dx) * kx * static_cast<float>(temp.xOffset) /
              static_cast<float>(imageWidth);
    yOffset = position[2] * (-2 * dx) * ky * static_cast<float>(temp.yOffset) /
              static_cast<float>(imageHeight);

    // calc character size in uv
    characterLength = glm::vec2((2 * dx) * realWidth * kx * position[2],
                                (2 * dx) * realHeight * ky);
}

glm::vec2 Character2d::GetLastCharacterLength() {
    // return glm::vec2((2*dx)*realWidth*kx*size,(2*dx)*realHeight*ky);
    return characterLength;
}
