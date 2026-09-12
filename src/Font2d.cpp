#include "Font2d.hpp"

#include "stb_image.h"

Character2d::Character2d() {}

Character2d::~Character2d() {}

bool Character2d::Init(shared_ptr<Material> _mat, string _fileName) {
    mesh.Init(_mat, "models/quad2x2front.ho3d");
    this->mat = _mat;

    // TODO implement font loading
    fileName = _fileName;

    // open the font file for reading
    fstream fin;
    fin.open(fileName.c_str(), ios::in);
    // temporary variable for reading
    string inStr;

    // temporary variable for loading texture
    string imgFilename;

    // flag indicating that kerning data has been sent
    bool kerning = false;
    // flag indicating that character data has been sent
    bool data = false;
    while (1) {
        // reading line by line
        getline(fin, inStr);
        // until we reach the end of the file
        if (!fin.eof()) {
            // reading data
            if (data) {
                // stream for conversion
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
                // initialize data about the new character
                FontCharacter temp2 = FontCharacter(t1[0], t1[1], t1[2], t1[3],
                                                    t2[0], t2[1], t1[4], t1[5]);
                // save character data to the list
                fontInfo.insert(pair<unsigned int, FontCharacter>(code, temp2));
            }
            // filling in kerning info
            if (kerning) {
                stringstream sstr;
                sstr << inStr;
                uint16_t code1;
                sstr >> code1;
                uint16_t code2;
                float f1;
                sstr >> code2 >> f1;
                uint32_t code = ((uint32_t)code1 << 16) | ((uint32_t)code2);
                // saving kerning data
                kerningInfo.insert(pair<uint32_t, float>(code, f1));
            }
            // searching for texture name
            int t = inStr.find("textures: ");
            string temp("textures: ");
            // found texture name
            if (t == 0) {
                imgFilename = string("fonts/") + string(inStr, temp.length());
                printf("\nFont image is %s", imgFilename.c_str());
            }

            // searching for font name
            t = inStr.find("px");
            if (t >= 0) {
                int t2 = inStr.find(" ");
                fontName = string(inStr, 0, t2);
                stringstream sstr;
                temp = string(inStr, t2, t - t2);
                sstr << temp;
                sstr >> fontHeight;

                // flag indicating data reading has started
                data = true;
                printf("\nFont name is %s", fontName.c_str());
                printf("\nFont height is %d", fontHeight);
            }
            // searching for information that kerning will be processed now
            t = inStr.find("kerning pairs:");
            if (t >= 0) {
                // stopped reading data
                data = false;
                // started reading about kerning pairs
                kerning = true;
            }

        } else
            break;
        inStr.clear();
    }  // filled font data

    GLuint texBufferID = 0;

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

    // load into material
    auto temp = std::make_shared<Texture2D>(texBufferID);
    _mat->SetColorTexture(temp);

    // get image dimensions
    FILE* imageFile = fopen(imgFilename.c_str(), "rb");
    char buffer2[4];

    // load header
    // TODO: understand this section
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

    // calculate width
    imageWidth = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
                 ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] << 24);
    printf("\n width=%d", imageWidth);

    // calculate height
    fread(&buffer2, 1, 4, imageFile);
    // printf("\nBuffer 5 %x %x %x
    // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    imageHeight = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
                  ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] << 24);
    printf("\n height=%d", imageHeight);

    // conversion of pixel to relative coordinates
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

void Character2d::Render(const Camera& cam) {
    SetAspectRatio(cam.GetWidth(), cam.GetHeight());
    this->mat->Use();

    // TODO idea: edit texture coordinates in the shader
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

    // TODO scaling???
    position[2] = position[2] / (float)fontHeight;
    temp = FontCharacter(0, 0, 0, 0, 0, 0, 0, 0);
    // find info about current character
    try {
        temp = fontInfo.at(currentCharacter);
    } catch (const std::out_of_range& oor) {
        // printf("\n char is out of range");
        characterLength = Vector2f(-1.0f, -1.0f);
        return;
    }
    // width and height in uv-coordinates
    realWidth = (float)temp.width / (float)imageWidth;
    realHeight = (float)temp.height / (float)imageHeight;
    dx = 1.0f;
    // texture coordinates positions
    xOffset =
        position[2] * (2 * dx) * kx * (float)temp.xOffset / (float)imageWidth;
    yOffset =
        position[2] * (-2 * dx) * ky * (float)temp.yOffset / (float)imageHeight;

    // TODO считаем длину символа в текстурных координатах??
    characterLength = Vector2f((2 * dx) * realWidth * kx * position[2],
                               (2 * dx) * realHeight * ky);
}

Vector2f Character2d::GetLastCharacterLength() {
    // return Vector2f((2*dx)*realWidth*kx*size,(2*dx)*realHeight*ky);
    return characterLength;
}
