#include "Font2d.h"

Character2d::Character2d()
{
}

Character2d::~Character2d()
{

}

bool Character2d::Init(shared_ptr<Material> _mat, string _fileName)
{
    mesh.Init(_mat,"Models/quad2x2front.ho3d");
    this->mat = _mat;

    //TODO сделать загрузку шрифта
    fileName = _fileName;

    //открываем файл шрифта на чтение
    fstream fin;
    fin.open(fileName.c_str(),ios::in);
    //временная переменная для чтения
    string inStr;

    //временная переменная для загрузки текстуры
    string imgFilename;

    //флаг того, что пошли данные о кернинге
    bool kerning=false;
    //флаг того, что пошли данные о символах
    bool data=false;
    while(1)
    {
        //читаем построчно
        getline(fin, inStr);
        //пока не достигнем конца файлв
        if(!fin.eof())
        {
            //чтение данных
             if(data)
            {
                //поток для конвертации
                stringstream sstr;
                //???
                unsigned int t1[6];
                //???
                int t2[2];
                sstr<<inStr;
                unsigned int code;
                sstr>>code;
                sstr>>t1[0]>>t1[1]>>t1[2]>>t1[3]>>t2[0]>>t2[1]>>t1[4]>>t1[5];
                //инициализируем данные о новой букве
                FontCharacter temp2=FontCharacter(t1[0],t1[1],t1[2],t1[3],t2[0],t2[1],t1[4],t1[5]);
                //сохраняем данные о букве в список
                fontInfo.insert( pair<unsigned int,FontCharacter>(code,temp2) ) ;
            }
            //заполняем инфу о кернинге
            if(kerning)
            {
                stringstream sstr;
                sstr<<inStr;
                uint16_t code1;
                sstr>>code1;
                uint16_t code2;
                float f1;
                sstr>>code2>>f1;
                uint32_t code=((uint32_t)code1<<16)|((uint32_t)code2);
                //сохраняем данные о кернинге
                kerningInfo.insert( pair<uint32_t,float>(code,f1));
            }
            //ищем название текстуры
            int t= inStr.find("textures: ");
            string temp("textures: ");
            //нашли название текстуры
            if(t==0)
            {
                imgFilename=string("Fonts/")+string(inStr,temp.length());
                printf("\nFont image is %s",imgFilename.c_str());
            }

            //ищем название шрифта
            t= inStr.find("px");
            if(t>=0)
            {
                int t2= inStr.find(" ");
                fontName=string(inStr,0,t2);
                stringstream sstr;
                temp=string(inStr,t2,t-t2);
                sstr<<temp;
                sstr>>fontHeight;

                //флаг о том, что сейчас будут читаться данные
                data=true;
                printf("\nFont name is %s",fontName.c_str());
                printf("\nFont height is %d",fontHeight);
            }
            //ищем информацию о том, что сейчас будет кернинг
            t= inStr.find("kerning pairs:");
            if(t>=0)
            {
                //перестали читать данные
                data=false;
                //начали читать о парах кернинга
                kerning=true;
            }

        }
        else
            break;
        inStr.clear();
    }//заполнили данные о шрифте


    GLuint texBufferID;
    //создаём текстуру
    texBufferID=SOIL_load_OGL_texture
                (
                    (char*)imgFilename.c_str(),
                    SOIL_LOAD_AUTO,
                    SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
                );
    //загружаем в материал
    shared_ptr<Texture2D> temp(new Texture2D(texBufferID));
    _mat->SetColorTexture( temp);

    //получаем размеры изображения
    FILE* imageFile=fopen(imgFilename.c_str(), "rb");
    char buffer2[4];

    //загружаем заголовочник
    //TODO разобраться в этой хуите
    fread(&buffer2,1,4,imageFile);
    //printf("\nBuffer 0 %x",buffer);
    //printf("\nBuffer 0 %x %x %x %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    fread(&buffer2,1,4,imageFile);
    //printf("\nBuffer 1 %x %x %x %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    fread(&buffer2,1,4,imageFile);
    //printf("\nIHDR chank");
    //printf("\nBuffer 2 %x %x %x %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    fread(&buffer2,1,4,imageFile);
    //printf("\nBuffer 3 %x %x %x %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    fread(&buffer2,1,4,imageFile);
    //printf("\nBuffer 4 %x %x %x %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);

    //считаем собсно ширину
    imageWidth=((uint32_t)buffer2[3]<<0)|((uint32_t)buffer2[2]<<8)|((uint32_t)buffer2[1]<<16)|((uint32_t)buffer2[0]<<24);
    printf("\n width=%d",imageWidth);

    //считаем высоту
    fread(&buffer2,1,4,imageFile);
    //printf("\nBuffer 5 %x %x %x %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    imageHeight=((uint32_t)buffer2[3]<<0)|((uint32_t)buffer2[2]<<8)|((uint32_t)buffer2[1]<<16)|((uint32_t)buffer2[0]<<24);
    printf("\n height=%d",imageHeight);

    //конверсия пикселя в относительные координаты
    pkx=1.0f/(float)imageWidth;
    pky=1.0f/(float)imageHeight;

    fclose(imageFile);

    return true;
}

float Character2d::GetAspectRatio()
{
    return aratio;
}

void Character2d::SetAspectRatio(int _width, int _height)
{
    aratio= (float)_height/(float)_width;
    kx=(float)imageWidth/(float)_width;
    ky=(float)imageHeight/(float)_height;
}

void Character2d::Render(Camera* cam)
{
    SetAspectRatio(cam->GetWidth(), cam->GetHeight());
    this->mat->Use();

    //TODO идея: редактировать текстурные координаты в шейдере
     /*float vertices[]={0.0f,(-2*dx)*realHeight*ky,
					0.0f,0.0f,
					(2*dx)*realWidth*kx,(-2*dx)*realHeight*ky,
					(2*dx)*realWidth*kx,0.0f};

    float uvs[]={0.0f,-realHeight,
                      0.0f,0.0f,
                      realWidth,-realHeight,
					  realWidth,0.0f};*/

    //mesh->SetScale()
}


float Character2d::GetHeight(unsigned int c)
{
    FontCharacter temp=fontInfo.at(c);
    float realHeight=(float)temp.height/(float)imageHeight;
    float dx=1.0f;
    return (2*dx)*realHeight*ky;
}

float Character2d::GetWidth(unsigned int c)
{
    FontCharacter temp=fontInfo.at(c);
    float realWidth=(float)temp.width/(float)imageWidth;
    float dx=1.0f;
    return (2*dx)*realWidth*kx;
}

float Character2d::GetSpaceWidth()
{
    FontCharacter temp=fontInfo.at(34);
    float realWidth=(float)temp.origW/(float)imageWidth;
    float dx=1.0f;
    return (2*dx)*realWidth*kx;
}

int Character2d::GetFontHeight()
{
    return fontHeight;
}


void Character2d::SetCharacter(unsigned int c)
{
    currentCharacter = c;

    //TODO масштабирование???
    position[2]=position[2]/(float)fontHeight;
    temp = FontCharacter(0,0,0,0,0,0,0,0);
    //ищем инфу о текущем символе
    try
    {
        temp=fontInfo.at(currentCharacter);
    }
    catch(const std::out_of_range& oor)
    {
        //printf("\n char is out of range");
        characterLength = Vector2f(-1.0f,-1.0f);
        return;
    }
    //ширина и высота в uv-координатах
    realWidth=(float)temp.width/(float)imageWidth;
    realHeight=(float)temp.height/(float)imageHeight;
    dx=1.0f;
    //позиции на текстуре
    xOffset=position[2]*(2*dx)*kx*(float)temp.xOffset/(float)imageWidth;
    yOffset=position[2]*(-2*dx)*ky*(float)temp.yOffset/(float)imageHeight;

    //TODO считаем длину символа в текстурных координатах??
    characterLength = Vector2f((2*dx)*realWidth*kx*position[2],(2*dx)*realHeight*ky);
}

Vector2f Character2d::GetLastCharacterLength()
{
    //return Vector2f((2*dx)*realWidth*kx*size,(2*dx)*realHeight*ky);
    return characterLength;
}
