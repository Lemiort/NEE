#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <main.h>

using namespace std;
int width;
int height;


GLuint gWorldID, gCamViewID;
GLuint rotateID;
GLuint dirLightDirID, dirLightColID;
GLuint pointLightColID, pointLightIntID, pointLightPosID;
GLuint spotLightColID, spotLightDirID, spotLightCutoffID,spotLightPosID;
GLuint camtransID,camPosID;

DirectionalLight* directionalLight1;
PointLight* pointLight1,* pointLight2;
SpotLight* spotLight1;
Camera* pGameCamera = NULL;
float Scale;
GLfloat light[]= {0.0,1.0,-1.0,1.0};
int spfaces;
int spverts;
Line* xline;
Line* yline;
Line* zline;
Line* dirLightLine;
Mesh TestMesh,Plane,Cube;
SkyBox* skybox1;
TextLine2d* tline1,*tline2;
FontLine2d* fLine1;
PerlinNoise* noise1;
Billboard* bb1;
ShadowMapFBO* smfbo1;

Material* mainMaterial,* secondMaterial,*shadowMaterial;
Material* shadowMeshMaterial;
Material* DSGeometryPassMaterial;
Material* DSPointLightMaterial;
Material* DSDirectionalLightMaterial;
Material* DSSpotLightMaterial;
Texture2D* colorMap1, *colorMap2, *whiteTexture;
float fps;
int frameCount;
double lastTime;
int renderType;

struct Mouse
{
    bool rightButtonPressed;
    bool leftButtonPressed;
    double posX;
    double posY;
    double dx, dy;
    Mouse()
    {
        rightButtonPressed=false;
        leftButtonPressed=false;
        posX=0.0f; posY=0.0f;
        dx=0.0f; dy=0.0f;
    }
    void Update(double x, double y)
    {
        dx=x-posX;
        dy=y-posY;
        posX=x;
        posY=y;
    }
}mouse;

static void CalcFPS()
{
    double currentTime = glfwGetTime();
    ++frameCount;
    if (currentTime - lastTime >= 1.0)
        {
            fps = (double)frameCount/(currentTime - lastTime);
            lastTime += 1.0;
            frameCount = 0;
        }
}

static void ErrorCallback(int error, const char* description)
{
    fputs(description, stderr);
}
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if( action == GLFW_PRESS || action==GLFW_REPEAT)
        {
            if(key==GLFW_KEY_I) pointLight1->position[0]+=0.1;
            if(key==GLFW_KEY_K) pointLight1->position[0]-=0.1;
            if(key==GLFW_KEY_L) pointLight1->position[1]-=0.1;
            if(key==GLFW_KEY_J) pointLight1->position[1]+=0.1;
            if(key==GLFW_KEY_U) pointLight1->position[2]+=0.1;
            if(key==GLFW_KEY_O) pointLight1->position[2]-=0.1;
            if(key==GLFW_KEY_F5)
            {
                renderType+=1;
                renderType%=6;
            }
            if(key==GLFW_KEY_PRINT_SCREEN)
                {
                    time_t rawtime;
                      struct tm * timeinfo;
                      char buffer[80];

                      time (&rawtime);
                      timeinfo = localtime(&rawtime);

                      strftime(buffer,80,"Screenshots/Screenshot %d-%m-%Y %I.%M.%S.tga\0",timeinfo);
                   int result=SOIL_save_screenshot
                    (
                        buffer,
                        SOIL_SAVE_TYPE_TGA,
                        0, 0, width,height
                    );
                    if(result) cout<<"\n Screenshot saved as "<<buffer;
                }
            else pGameCamera->OnKeyboard(key);
        }
}
static void MousePosCallBack(GLFWwindow* window, double x, double y)
{
    mouse.Update(x,y);
    if(mouse.rightButtonPressed || true)
        pGameCamera->OnMouse(mouse.posX, mouse.posY);
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if(button==GLFW_MOUSE_BUTTON_2)//right button
    {
        if(action==GLFW_PRESS)
        {
            mouse.rightButtonPressed=true;
        }
        else
        {
            mouse.rightButtonPressed=false;
        }
    }
}

static void FrameBufferSizeCallback(GLFWwindow* window, int w, int h)
{
    width=w;
    height=h;
    if(tline1!=NULL)
        tline1->SetAspectRatio(width,height);
    if(fLine1!=NULL)
        fLine1->SetAspectRatio(width,height);
    if(smfbo1!=NULL)
        smfbo1->Init(w,h);
    if(gBuffer1!=NULL)
        gBuffer1->Init(w,h);
    glViewport(0, 0, width, height);
}

static void ShadowPass()
{
    smfbo1->BindForWriting();
    //gBuffer1->BindForWriting();
    //этап рисовки
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //CalcFPS();
    Scale += 0.021f;
    TestMesh.SetRotate(0,30*sinf(Scale),0);
    TestMesh.SetScale(0.02,0.02,0.02);
    TestMesh.SetPosition(0,-0.2,0);
    //light3->SetPos(pGameCamera->GetPos());
    //light3->SetDir(pGameCamera->GetTarget()-pGameCamera->GetPos());
    Camera* lightCam=new Camera(width,height,spotLight1->GetPos(),Vector3f(-1.0,-1.0,-1.0),Vector3f(0.0,1.0,0.0));

    Plane.SetScale(30.0f,30.0f,30.0f);
    Plane.SetPosition(0.0f,-3.0f,0.0f);
    Plane.SetRotate(0.0,0.0,0.0);
    Assistant TM;//TM - Для объекта, 2- для нормали объекта, 3 - для позиции камера для спекуляра
    TM.SetCamera(lightCam->GetPos(), lightCam->GetTarget(), lightCam->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    shadowShader->Use();
    gCamViewID =	shadowShader->GetUniformLocation("gVC");
    rotateID =	    shadowShader->GetUniformLocation("mRotate");
    camPosID =    shadowShader->GetUniformLocation("s_vCamPos");

    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    //освещение
    {

        //направленный свет
        Assistant LA;
        LA.Scale(directionalLight1->color[0],directionalLight1->color[1],directionalLight1->color[2]);
        glUniformMatrix4fv(dirLightColID,1, GL_TRUE, (const GLfloat*)LA.GetScaleTrans());
        glUniform3f(dirLightDirID,directionalLight1->direction[0],directionalLight1->direction[1],directionalLight1->direction[2]);
        //точечный
        Assistant LA2;
        LA2.Scale(pointLight1->color[0],pointLight1->color[1],pointLight1->color[2]);
        glUniformMatrix4fv(pointLightColID,1, GL_TRUE, (const GLfloat*)LA2.GetScaleTrans());
        glUniform3f(pointLightPosID,pointLight1->position[0],pointLight1->position[1],pointLight1->position[2]);
        glUniform1f(pointLightIntID,pointLight1->power);
        //прожектор
        Assistant LA3;
        LA3.Scale(spotLight1->color[0],spotLight1->color[1],spotLight1->color[2]);
        glUniformMatrix4fv(spotLightColID,1, GL_TRUE, (const GLfloat*)LA3.GetScaleTrans());
        glUniform3f(spotLightDirID,spotLight1->direction[0],spotLight1->direction[1],spotLight1->direction[2]);
        glUniform1f(spotLightCutoffID,cosf((spotLight1->Cutoff)*3.14/180.0f));
        glUniform3f(spotLightPosID,spotLight1->position[0],spotLight1->position[1],spotLight1->position[2]);
    }

    //вращение камеры для спекуляра
    glUniform3f(camPosID,lightCam->GetPos().x,lightCam->GetPos().y,lightCam->GetPos().z);

    Cube.SetMaterial(shadowMaterial);
    for(float i=-5.0f;i<5.0f;i+=0.1f)
        for(float j=-5.0f;j<5.0f;j+=0.1f)
    {
        Cube.SetScale(0.05f,0.05f,0.05f);
        Cube.SetRotate(0,30*sinf(Scale),0);
        Cube.SetPosition(i,noise1->GetHeight(i,j),j);
        Cube.Render(30,width, height, 1, 1000,lightCam);
    }
    Plane.SetMaterial(shadowMaterial);
    Plane.Render(30,width, height, 1, 1000,lightCam);

    delete lightCam;
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

static void RenderPass()
{
    //этап рисовки
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //CalcFPS();
    Scale += 0.021f;
    TestMesh.SetRotate(0,30*sinf(Scale),0);
    TestMesh.SetScale(0.02,0.02,0.02);
    TestMesh.SetPosition(0,-0.2,0);


    Plane.SetScale(30.0f,30.0f,30.0f);
    Plane.SetPosition(0.0f,-3.0f,0.0f);
    Plane.SetRotate(0.0,0.0,0.0);
    Assistant TM,TM2;//TM - Для объекта, 2- для нормали объекта, 3 - для позиции камера для спекуляра
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    Camera* lightCam=new Camera(width,height,spotLight1->GetPos(),Vector3f(-1.0,-1.0,-1.0),Vector3f(0.0,1.0,0.0));

    TM2.SetCamera(lightCam->GetPos(), lightCam->GetTarget(), lightCam->GetUp());
    TM2.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);


    /*meshShader->Use();
    gCamViewID =	meshShader->GetUniformLocation("gVC");
    rotateID =	    meshShader->GetUniformLocation("mRotate");
    camPosID =    meshShader->GetUniformLocation("s_vCamPos");


    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    //освещение
    {

        //направленный свет
        Assistant LA;
        LA.Scale(light1->color[0],light1->color[1],light1->color[2]);
        glUniformMatrix4fv(dirLightColID,1, GL_TRUE, (const GLfloat*)LA.GetScaleTrans());
        glUniform3f(dirLightDirID,light1->direction[0],light1->direction[1],light1->direction[2]);
        //точечный
        Assistant LA2;
        LA2.Scale(light2->color[0],light2->color[1],light2->color[2]);
        glUniformMatrix4fv(pointLightColID,1, GL_TRUE, (const GLfloat*)LA2.GetScaleTrans());
        glUniform3f(pointLightPosID,light2->position[0],light2->position[1],light2->position[2]);
        glUniform1f(pointLightIntID,light2->power);
        //прожектор
        Assistant LA3;
        LA3.Scale(light3->color[0],light3->color[1],light3->color[2]);
        glUniformMatrix4fv(spotLightColID,1, GL_TRUE, (const GLfloat*)LA3.GetScaleTrans());
        glUniform3f(spotLightDirID,light3->direction[0],light3->direction[1],light3->direction[2]);
        glUniform1f(spotLightCutoffID,cosf((light3->Cutoff)*3.14/180.0f));
        glUniform3f(spotLightPosID,light3->position[0],light3->position[1],light3->position[2]);
    }

    //вращение камеры для спекуляра
    glUniform3f(camPosID,pGameCamera->GetPos().x,pGameCamera->GetPos().y,pGameCamera->GetPos().z);*/

    /*Cube.SetMaterial(mainMaterial);
    for(float i=-5.0f;i<5.0f;i+=0.1f)
        for(float j=-5.0f;j<5.0f;j+=0.1f)
    {
        Cube.SetScale(0.001f,0.001f,0.001f);
        Cube.SetRotate(0,30*sinf(Scale),0);
        Cube.SetPosition(i,noise1->GetHeight(i,j),j);
        Cube.Render(30,width, height, 1, 1000,pGameCamera);
    }*/

   /* TestMesh.Render(30,width, height, 1, 1000,pGameCamera);
     TestMesh.SetPosition(-3,-0.2,0);
     TestMesh.Render(30,width, height, 1, 1000,pGameCamera);*/
    skybox1->Render(30,width,height, 1, 1000,pGameCamera);



    /*Texture2D* tempTexture=new Texture2D(gBuffer1->GetTexture(2),false);
    //shadowMeshMaterial->SetShadowTexture(tempTexture);
    mainMaterial->SetColorTexture(tempTexture);
    fLine1->Render(tempTexture->GetParameters(),-1.0f,-0.2f,32.0f);*/
    Plane.SetMaterial(mainMaterial);
    //тупо копипаст
    {
        meshShader->Use();
        GLuint gLightCamViewID=meshShader->GetUniformLocation("gLightVC");
        gCamViewID =	meshShader->GetUniformLocation("gVC");
        rotateID =	    meshShader->GetUniformLocation("mRotate");
        camPosID =    meshShader->GetUniformLocation("s_vCamPos");

        glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
        glUniformMatrix4fv(gLightCamViewID, 1, GL_TRUE, (const GLfloat*)TM2.GetVC());

        //освещение
        {

            //направленный свет
            Assistant LA;
            LA.Scale(directionalLight1->color[0],directionalLight1->color[1],directionalLight1->color[2]);
            glUniformMatrix4fv(dirLightColID,1, GL_TRUE, (const GLfloat*)LA.GetScaleTrans());
            glUniform3f(dirLightDirID,directionalLight1->direction[0],directionalLight1->direction[1],directionalLight1->direction[2]);
            //точечный
            Assistant LA2;
            LA2.Scale(pointLight1->color[0],pointLight1->color[1],pointLight1->color[2]);
            glUniformMatrix4fv(pointLightColID,1, GL_TRUE, (const GLfloat*)LA2.GetScaleTrans());
            glUniform3f(pointLightPosID,pointLight1->position[0],pointLight1->position[1],pointLight1->position[2]);
            glUniform1f(pointLightIntID,pointLight1->power);
            //прожектор
            Assistant LA3;
            LA3.Scale(spotLight1->color[0],spotLight1->color[1],spotLight1->color[2]);
            glUniformMatrix4fv(spotLightColID,1, GL_TRUE, (const GLfloat*)LA3.GetScaleTrans());
            glUniform3f(spotLightDirID,spotLight1->direction[0],spotLight1->direction[1],spotLight1->direction[2]);
            glUniform1f(spotLightCutoffID,cosf((spotLight1->Cutoff)*3.14/180.0f));
            glUniform3f(spotLightPosID,spotLight1->position[0],spotLight1->position[1],spotLight1->position[2]);
        }

        //вращение камеры для спекуляра
        glUniform3f(camPosID,pGameCamera->GetPos().x,pGameCamera->GetPos().y,pGameCamera->GetPos().z);
    }
    //Plane.SetMaterial(mainMaterial);
    Plane.Render(30,width, height, 1, 1000,pGameCamera);
    Cube.SetMaterial(mainMaterial);
    for(float i=-5.0f;i<5.0f;i+=0.1f)
        for(float j=-5.0f;j<5.0f;j+=0.1f)
    {
        Cube.SetScale(0.05f,0.05f,0.05f);
        Cube.SetRotate(0,30*sinf(Scale),0);
        Cube.SetPosition(i,noise1->GetHeight(i,j),j);
        Cube.Render(30,width, height, 1, 1000,pGameCamera);
    }
    //delete tempTexture;

    bb1->Render(30,width, height, 1, 1000,pGameCamera);


   /* xline->Render(pGameCamera,width, height);
    yline->Render(pGameCamera, width, height);
    zline->Render(pGameCamera,width, height);

   // tline2->Render(-1,0.9,64.0,(char*)(ConvertToString(Cube.GetNumFaces()*10000)+" faces").c_str());
    //tline1->Render(-1,0.5,32.0,(char*)ConvertToString(fps).c_str());
    //fLine1->Render(ConvertToString(fps).c_str(),-1.0f,0.9f,30.0f);
    spfaces=Cube.GetNumFaces()*100*100+TestMesh.GetNumFaces()+Plane.GetNumFaces();
    fLine1->Render((ConvertToString(spfaces)+" faces").c_str(),-1.0f,0.0f,45.0f);*/
    delete lightCam;
}

static void DSBeginLightPasses()
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    //gBuffer1->BindForReading();
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
}

static void DSLightingPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gBuffer1->BindForReading();

   // GLsizei HalfWidth = (GLsizei)(width / 2.0f);
    //GLsizei HalfHeight = (GLsizei)(height / 2.0f);

    //gBuffer1->CheckTextures();
    /*
    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
    glBlitFramebuffer(0, 0,width, height, 0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, width, height, 0, HalfHeight, HalfWidth, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, width, height, HalfWidth, HalfHeight, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
    glBlitFramebuffer(0, 0, width, height, HalfWidth, 0, width, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);*/
    switch(renderType)
    {
        case 1:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
            glBlitFramebuffer(0, 0,width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        break;
        case 2:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
            glBlitFramebuffer(0, 0,width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        break;
         case 3:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
            glBlitFramebuffer(0, 0,width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        break;
         case 4:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
            glBlitFramebuffer(0, 0,width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        break;
    }

   /* fLine1->Render(ConvertToString(fps).c_str(),-1.0f,0.9f,30.0f);
    spfaces=Cube.GetNumFaces()*100*100+TestMesh.GetNumFaces()+Plane.GetNumFaces();
    fLine1->Render((ConvertToString(spfaces)+" faces").c_str(),-1.0f,0.0f,45.0f);*/
}

static void DSPointLightPass()
{
    gBuffer1->BindForReading();


    Assistant TM;//TM - Для объекта, 2- для нормали объекта, 3 - для позиции камера для спекуляра
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    //включаем шейдер
    DSPointLightShader->Use();

    //определяем адрес переменных камеры
    gCamViewID =	DSPointLightShader->GetUniformLocation("gVC");
    rotateID =	    DSPointLightShader->GetUniformLocation("mRotate");
    camPosID =    DSPointLightShader->GetUniformLocation("s_vCamPos");

    //загружаем матрицу камеры
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    //взагружаем вращение камеры для спекуляра
    glUniform3f(camPosID,pGameCamera->GetPos().x,pGameCamera->GetPos().y,pGameCamera->GetPos().z);


    //загружаем текстуры в шейдер
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(0),4);//world pos
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(1),5);//diffuse
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(2),6);//normal
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(3),7);//UV

    //включаем данные из буффера
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glClear(/*GL_COLOR_BUFFER_BIT |*/GL_DEPTH_BUFFER_BIT);


    //определяем адрес параметров света
    pointLightPosID = DSPointLightShader->GetUniformLocation("s_vPointLightPos");
    pointLightIntID = DSPointLightShader->GetUniformLocation("pointLightIntensity");
    pointLightColID = DSPointLightShader->GetUniformLocation("pointLightColor");

    //загружаем параметры света для источника 1
    Assistant LA2;
    LA2.Scale(pointLight1->color[0],pointLight1->color[1],pointLight1->color[2]);
    glUniformMatrix4fv(pointLightColID,1, GL_TRUE, (const GLfloat*)LA2.GetScaleTrans());
    glUniform3f(pointLightPosID,pointLight1->position[0],pointLight1->position[1],pointLight1->position[2]);
    glUniform1f(pointLightIntID,pointLight1->power);

    pointLight1->Render(30,width,height,1.0,1000.0,pGameCamera);

    //загружаем параметры для источника 2
    LA2.Scale(pointLight2->color[0],pointLight2->color[1],pointLight2->color[2]);
    glUniformMatrix4fv(pointLightColID,1, GL_TRUE, (const GLfloat*)LA2.GetScaleTrans());
    glUniform3f(pointLightPosID,pointLight2->position[0],pointLight2->position[1],pointLight2->position[2]);
    glUniform1f(pointLightIntID,pointLight2->power);

    pointLight2->Render(30,width,height,1.0,1000.0,pGameCamera);
}

static int DSDirectionalLightPass()
{
    gBuffer1->BindForReading();
    glDisable(GL_DEPTH_TEST);

    Assistant TM;//TM - Для объекта, 2- для нормали объекта, 3 - для позиции камера для спекуляра
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    DSDirectionalLightShader->Use();
    //получаем адрес параметров камеры
    gCamViewID =	DSDirectionalLightShader->GetUniformLocation("gVC");
    rotateID =	    DSDirectionalLightShader->GetUniformLocation("mRotate");
    camPosID =    DSDirectionalLightShader->GetUniformLocation("s_vCamPos");
    //загружаем вращение камеры для спекуляра
    glUniform3f(camPosID,pGameCamera->GetPos().x,pGameCamera->GetPos().y,pGameCamera->GetPos().z);
    //загружаем матрицу камеры
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());

    //получаем адрес переменных света
    dirLightColID = DSDirectionalLightShader->GetUniformLocation("dirLightColor");
    dirLightDirID = DSDirectionalLightShader->GetUniformLocation("dirLightDirection");

    //загрузка параметров света
    Assistant LA2;
    LA2.Scale(directionalLight1->color[0],directionalLight1->color[1],directionalLight1->color[2]);
    glUniformMatrix4fv(dirLightColID,1, GL_TRUE, (const GLfloat*)LA2.GetScaleTrans());
    glUniform3f(dirLightDirID,directionalLight1->direction[0],
                            directionalLight1->direction[1],
                            directionalLight1->direction[2]);

    //загружаем текстуры в шейдер
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(0),4);//world pos
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(1),5);//diffuse
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(2),6);//normal
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(3),7);//UV



    /*printf("\n %f %f %f",directionalLight1->direction[0],
                            directionalLight1->direction[1],
                            directionalLight1->direction[2]);*/
    //glUniform1f(pointLightIntID,pointLight1->power);

    //включаем данные из буффера
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);

    directionalLight1->Render(30,width,height,1.0,1000.0,pGameCamera);
    glEnable(GL_DEPTH_TEST);
    return 0;
}

static void InterfacePass()
{
    glDisable(GL_DEPTH_TEST);
    xline->Render(pGameCamera,width, height);
    yline->Render(pGameCamera, width, height);
    zline->Render(pGameCamera,width, height);
    std::string strCampos = ConvertToString(pGameCamera->GetPos().x)+"; "+ConvertToString(pGameCamera->GetPos().y)
    +"; "+ConvertToString(pGameCamera->GetPos().z);
    fLine1->Render((strCampos).c_str(),-1.0f,0.0f,24.0f);
    //gBuffer1->CheckTextures();
    CalcFPS();
    fLine1->Render(ConvertToString(fps),-1.0f,-0.1f,24.0f);

    if(renderType==0)
    {
        fLine1->Render("Deferred shading",-1.0f,-0.2f,24.0f);
    }

    pointLight1->Render(30,width, height, 1, 1000,pGameCamera);
    dirLightLine->Render(pGameCamera,width,height);
    glEnable(GL_DEPTH_TEST);
}

static void DSGeometryPass()
{

    gBuffer1->BindForWriting();
    //запись в глубину
    //glDepthMask(GL_TRUE);

    //этап рисовки
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glEnable(GL_DEPTH_TEST);
    //glDisable(GL_BLEND);

    CalcFPS();

    Assistant TM;//TM - Для объекта, 2- для нормали объекта, 3 - для позиции камера для спекуляра
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    DSGeometryPassShader->Use();
    gCamViewID =	DSGeometryPassShader->GetUniformLocation("gVC");

    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());

    Cube.SetMaterial(DSGeometryPassMaterial);
    for(float i=-5.0f;i<5.0f;i+=0.1f)
        for(float j=-5.0f;j<5.0f;j+=0.1f)
    {
        Cube.SetScale(0.05f,0.05f,0.05f);
        Cube.SetRotate(0,30*sinf(Scale),0);
        Cube.SetPosition(i,noise1->GetHeight(i,j),j);
        Cube.Render(30,width, height, 1, 1000,pGameCamera);
    }

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    //glDepthMask(GL_FALSE);
    //glDisable(GL_DEPTH_TEST);

}

static void RenderScene(GLFWwindow* window)
{
    //ShadowPass();
    //RenderPass();
    //DSGeometryPass();
   // DSBeginLightPasses();//вообще пока не нужно
    //DSPointLightPass();
    //DSLightingPass();
    //обычный deffered shading
    if(renderType==0)
    {
        DSGeometryPass();
        DSBeginLightPasses();//вообще пока не нужно
        DSPointLightPass();
        DSDirectionalLightPass();
    }
    //дебагинговый вид
    else if(renderType <=4)
    {
        DSGeometryPass();
        DSLightingPass();
    }
    else
    {
        RenderPass();
    }
    InterfacePass();
}
static int InitScene()
{
    Scale=0;
    lastTime = glfwGetTime();
    frameCount=0;

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);

    //нормальный шейдер
    char* vertexShaderSorceCode=ReadFile("Shaders/vertexShader.vsh");
    char* fragmentShaderSourceCode=ReadFile("Shaders/fragmentShader.fsh");
    {
        meshShader=new Shader();
        meshShader->AddShader((const char*)vertexShaderSorceCode,VertexShader);
        meshShader->AddShader((const char*)fragmentShaderSourceCode,FragmnetShader);
        meshShader->Init();
        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;

        gCamViewID =	meshShader->GetUniformLocation("gVC");
        rotateID =	    meshShader->GetUniformLocation("mRotate");
        camPosID =    meshShader->GetUniformLocation("s_vCamPos");
    }

    //основной материал
    {
        mainMaterial = new Material;
        mainMaterial->Init(meshShader);
    }

    //Cube.Init(mainMaterial,"Models/sphere2.ho3d");
    TestMesh.Init(mainMaterial,"Models/torus1.ho3d");
    //Plane.Init(mainMaterial, "Models/plane.ho3d");



    //шейдер тени
    {
        vertexShaderSorceCode=ReadFile("Shaders/fbo.vsh");
        fragmentShaderSourceCode=ReadFile("Shaders/fbo.fsh");
        shadowShader=new Shader();
        shadowShader->AddShader((const char*)vertexShaderSorceCode,VertexShader);
        shadowShader->AddShader((const char*)fragmentShaderSourceCode,FragmnetShader);
        shadowShader->Init();

        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;
    }

    //материал тени
    {
        shadowMaterial = new Material;
        shadowMaterial->Init(shadowShader);
    }


    //затенённый шейдер
    {
        vertexShaderSorceCode=ReadFile("Shaders/shadowed.vsh");
        //fragmentShaderSourceCode=ReadFile("Shaders/fragmentShader.fsh");
        fragmentShaderSourceCode=ReadFile("Shaders/shadowed.fsh");
        shadowMeshShader=new Shader();
        shadowMeshShader->AddShader((const char*)vertexShaderSorceCode,VertexShader);
        shadowMeshShader->AddShader((const char*)fragmentShaderSourceCode,FragmnetShader);
        shadowMeshShader->Init();

        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;
    }

    //затенённый материал
    {
        shadowMeshMaterial = new Material;
        shadowMeshMaterial->Init(shadowMeshShader);
        whiteTexture=new Texture2D;
        whiteTexture->Load("Textures/white.png");
        shadowMeshMaterial->SetColorTexture(whiteTexture);
        Cube.Init(shadowMeshMaterial,"Models/normal_sphere.ho3d");
        Plane.Init(shadowMeshMaterial, "Models/normal_plane.ho3d");
    }

    //шейдер GBufferа и сам буффер, материал
    {
        gBuffer1 = new GBuffer;
        gBuffer1->Init(width,height);
        //грузим шейдер
        {
            vertexShaderSorceCode=ReadFile("Shaders/DSGeometryPass.vsh");
            fragmentShaderSourceCode=ReadFile("Shaders/DSGeometryPass.fsh");
            DSGeometryPassShader=new Shader();
            DSGeometryPassShader->AddShader((const char*)vertexShaderSorceCode,VertexShader);
            DSGeometryPassShader->AddShader((const char*)fragmentShaderSourceCode,FragmnetShader);
            DSGeometryPassShader->Init();

            delete[] vertexShaderSorceCode;
            delete[] fragmentShaderSourceCode;
        }
        //грузим материал
        {
            DSGeometryPassMaterial = new Material;
            DSGeometryPassMaterial->Init(DSGeometryPassShader);
        }

    }

    //шейдера света
    {
        //шейдер точечного света
        {
            //грузим шейдер
            {
                vertexShaderSorceCode=ReadFile("Shaders/DSPointLight.vsh");
                fragmentShaderSourceCode=ReadFile("Shaders/DSPointLight.fsh");
                DSPointLightShader=new Shader();
                DSPointLightShader->AddShader((const char*)vertexShaderSorceCode,VertexShader);
                DSPointLightShader->AddShader((const char*)fragmentShaderSourceCode,FragmnetShader);
                DSPointLightShader->Init();

                delete[] vertexShaderSorceCode;
                delete[] fragmentShaderSourceCode;
            }
            //грузим материал
            {
                DSPointLightMaterial = new Material;
                DSPointLightMaterial->Init(DSPointLightShader);
            }
        }

        //шейдер направленного света
        {
            //грузим шейдер
            {
                vertexShaderSorceCode=ReadFile("Shaders/DSDirectionalLight.vsh");
                fragmentShaderSourceCode=ReadFile("Shaders/DSDirectionalLight.fsh");
                DSDirectionalLightShader=new Shader();
                DSDirectionalLightShader->AddShader((const char*)vertexShaderSorceCode,VertexShader);
                DSDirectionalLightShader->AddShader((const char*)fragmentShaderSourceCode,FragmnetShader);
                DSDirectionalLightShader->Init();

                delete[] vertexShaderSorceCode;
                delete[] fragmentShaderSourceCode;
            }
            //грузим материал
            {
                DSDirectionalLightMaterial = new Material;
                DSDirectionalLightMaterial->Init(DSDirectionalLightShader);
            }
        }

        //шейдер прожектора света
        {
            //грузим шейдер
            {
                vertexShaderSorceCode=ReadFile("Shaders/DSSpotLight.vsh");
                fragmentShaderSourceCode=ReadFile("Shaders/DSSpotLight.fsh");
                DSSpotLightShader=new Shader();
                DSSpotLightShader->AddShader((const char*)vertexShaderSorceCode,VertexShader);
                DSSpotLightShader->AddShader((const char*)fragmentShaderSourceCode,FragmnetShader);
                DSSpotLightShader->Init();

                delete[] vertexShaderSorceCode;
                delete[] fragmentShaderSourceCode;
            }
            //грузим материал
            {
                DSSpotLightMaterial = new Material;
                DSSpotLightMaterial->Init(DSSpotLightShader);
            }
        }
    }

    //шейдер скайбокса
    {


    vertexShaderSorceCode=ReadFile("Shaders/skybox.vsh");
    fragmentShaderSourceCode=ReadFile("Shaders/skybox.fsh");

    skyboxShader=new Shader();

    skyboxShader->AddShader(vertexShaderSorceCode, VertexShader);
    skyboxShader->AddShader(fragmentShaderSourceCode, FragmnetShader);
    skyboxShader->Init();
    delete[] vertexShaderSorceCode;
    delete[] fragmentShaderSourceCode;
    }

    //шейдер текста
    {
    vertexShaderSorceCode=ReadFile("Shaders/text2d.vsh");
    fragmentShaderSourceCode=ReadFile("Shaders/text2d.fsh");


    textShader=new Shader();
    textShader->AddShader(vertexShaderSorceCode,VertexShader);
    textShader->AddShader(fragmentShaderSourceCode,FragmnetShader);
    textShader->Init();
    delete[] vertexShaderSorceCode;
    delete[] fragmentShaderSourceCode;
    }

    skybox1= new SkyBox(skyboxShader);
    skybox1->Init("Textures",
                  "sp3right.tga",
                  "sp3left.tga",
                  "sp3top.tga",
                  "sp3bot.tga",
                  "sp3front.tga",
                  "sp3back.tga");
    //настройка света и единичных векторов
    {
        dirLightDirID=		meshShader->GetUniformLocation("dLightDir");
        dirLightColID=		meshShader->GetUniformLocation("dLightCol");
        spotLightDirID=		meshShader->GetUniformLocation("sLightDir");
        spotLightColID=		meshShader->GetUniformLocation("sLightCol");
        spotLightCutoffID=	meshShader->GetUniformLocation( "sLightCutoff");
        spotLightPosID=		meshShader->GetUniformLocation("sLightPos");

        pointLightIntID=	meshShader->GetUniformLocation("pLightInt");
        pointLightColID=	meshShader->GetUniformLocation("pLightCol");
        pointLightPosID=	meshShader->GetUniformLocation("pLightPos");

        directionalLight1=new DirectionalLight(-1.5f,-1.0f,-1.5f,//direction
                                     0.5f,0.5f,0.5f,//color
                                     DSDirectionalLightMaterial);
        pointLight1=new PointLight(0,1,-0.2, 1.0,1.0,1.0,0.1, DSPointLightMaterial);
        pointLight2=new PointLight(1.5,0.4,0.0,
                                   0.2, 1.0,6.0,
                                   0.1,
                                    DSPointLightMaterial);
        spotLight1=new SpotLight(-1.0f,-1.0f,-1.0f,//direction
                     1.0f,1.0f,0.0f,//color
                     2.0f,2.0f,2.0f,//position
                     35.0f,
                     DSSpotLightMaterial);
        Vector3f PX(1,0,0);
        Vector3f PY(0,1,0);
        Vector3f PZ(0,0,1);
        Vector3f P0(0,0,0);

        xline= new Line(PX,P0,PX);
        yline= new Line(PY,P0,PY,xline->GetShader());
        zline= new Line(PZ,P0,PZ,xline->GetShader());
        dirLightLine=new Line(P0,directionalLight1->GetDir(),directionalLight1->GetCol());
    }

    bb1=new Billboard();
    bb1->Init("Textures/monster_hellknight.png");
    bb1->SetPos(Vector3f(0,0,0));

    noise1=new PerlinNoise(1,10.3,0.5,2,42);

    tline1=new TextLine2d();
    tline2=new TextLine2d();
    fLine1=new FontLine2d();
    tline1->Init(width,height,textShader);
    tline2->Init(width,height,textShader);
    fLine1->Init(string("Fonts/MagistralIC_UTF-8.fnt"),textShader);
    fLine1->SetAspectRatio(width,height);

    //*************Shadow MAP FBO**********/
    smfbo1 = new ShadowMapFBO();
    smfbo1->Init(width,height);
    //************************************/
    //pGameCamera = new Camera(width,height,light3->GetPos(),Vector3f(-1.0,-1.0,-1.0),Vector3f(0.0,1.0,0.0));
    return 0;
}
int main(void)
{
    renderType = 0;
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL
    GLFWwindow* window;
    glfwSetErrorCallback(ErrorCallback);
    //glewExperimental = true; // Needed for core profile
    if (!glfwInit())
        exit(EXIT_FAILURE);
    //заголовок
    string title("HOGL ");
    title+= AutoVersion::STATUS;
    title+=" ";
    title+=AutoVersion::FULLVERSION_STRING;
    title+=" build ";
    title+=ConvertToString(AutoVersion::BUILDS_COUNT);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(), NULL, NULL);
    if (!window)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallback);
    width=WINDOW_WIDTH;
    height=WINDOW_HEIGHT;
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window,MousePosCallBack);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    GLenum res = glewInit();
    if (res != GLEW_OK)
        {
            fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
            return 1;
        }
    else
    {
        printf("\nGLEW status is %d \n", res);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //включаем прозрачность
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(InitScene()!=0)
        return -1;
    glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);


    while (!glfwWindowShouldClose(window))
        {
            RenderScene(window);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
