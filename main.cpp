#include <GL/glew.h>
#include <GL/glfw3.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <main.h>
#include <sstream>
using namespace std;
int width;
int height;
GLuint VBO, FRBO;
GLuint depthRBO;
GLuint gWorldID, gCamViewID;
GLuint rotateID;
GLuint dirLightDirID, dirLightColID;
GLuint pointLightColID, pointLightIntID, pointLightPosID;
GLuint spotLightColID, spotLightDirID, spotLightCutoffID,spotLightPosID;
GLuint camtransID,camPosID;
GLuint fboTexID;
DirectionalLight* light1;
PointLight* light2;
SpotLight* light3;
GLuint shaderProgramID, shaderProgramID2;
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
ShadowMapFBO smfbo1;
float fps;
int frameCount;
double lastTime;

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
std::string ConvertToString (float number)
{
    std::ostringstream buff;
    buff<<number;
    return buff.str();
}
std::string ConvertToString(int number)
{
    std::ostringstream buff;
    buff<<number;
    return buff.str();
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
            if(key==GLFW_KEY_I) light2->position[0]+=0.1;
            if(key==GLFW_KEY_K) light2->position[0]-=0.1;
            if(key==GLFW_KEY_L) light2->position[1]-=0.1;
            if(key==GLFW_KEY_J) light2->position[1]+=0.1;
            if(key==GLFW_KEY_U) light2->position[2]+=0.1;
            if(key==GLFW_KEY_O) light2->position[2]-=0.1;
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
    glViewport(0, 0, width, height);
}
static void RenderScene(GLFWwindow* window)
{
    //этап теней
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shadowShader->shaderProgramID);
     gCamViewID =	shadowShader->GetUniformLocation("gVC");
    rotateID =	    shadowShader->GetUniformLocation("mRotate");
    camPosID =    shadowShader->GetUniformLocation("s_vCamPos");
    smfbo1.BindForWriting();
    Cube.SetShader(shadowShader);
    //вращение камеры дл€ спекул€ра
    glUniform3f(camPosID,pGameCamera->GetPos().x,pGameCamera->GetPos().y,pGameCamera->GetPos().z);
    for(float i=-5.0f;i<5.0f;i+=0.1f)
        for(float j=-5.0f;j<5.0f;j+=0.1f)
    {
        Cube.SetScale(0.001f,0.001f,0.001f);
        Cube.SetRotate(0,30*sinf(Scale),0);
        Cube.SetPosition(i,noise1->GetHeight(i,j),j);
        Cube.Render(30,width, height, 1, 1000,pGameCamera);
    }


    //этап рисовки
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    CalcFPS();
    Scale += 0.021f;
    TestMesh.SetRotate(0,30*sinf(Scale),0);
    TestMesh.SetScale(0.02,0.02,0.02);
    TestMesh.SetPosition(0,-0.2,0);


    Plane.SetScale(0.3,0.3,0.3);
    Plane.SetPosition(0.0,-3,-3);
    Plane.SetRotate(0.0,0.0,0.0);
    Assistant TM;//TM - ƒл€ объекта, 2- дл€ нормали объекта, 3 - дл€ позиции камера дл€ спекул€ра
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    glUseProgram(shaderProgramID);
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

    //вращение камеры дл€ спекул€ра
    Cube.SetShader(meshShader);
    glUniform3f(camPosID,pGameCamera->GetPos().x,pGameCamera->GetPos().y,pGameCamera->GetPos().z);
    for(float i=-5.0f;i<5.0f;i+=0.1f)
        for(float j=-5.0f;j<5.0f;j+=0.1f)
    {
        Cube.SetScale(0.001f,0.001f,0.001f);
        Cube.SetRotate(0,30*sinf(Scale),0);
        Cube.SetPosition(i,noise1->GetHeight(i,j),j);
        Cube.Render(30,width, height, 1, 1000,pGameCamera);
    }

    TestMesh.Render(30,width, height, 1, 1000,pGameCamera);
     TestMesh.SetPosition(-3,-0.2,0);
     TestMesh.Render(30,width, height, 1, 1000,pGameCamera);
    skybox1->Render(30,width,height, 1, 1000,pGameCamera);

   //  smfbo1.BindForReading(GL_TEXTURE3);
   // Plane.SetTexture(3);

    Plane.Render(30,width, height, 1, 1000,pGameCamera);
    bb1->Render(30,width, height, 1, 1000,pGameCamera);

    light2->Render(pGameCamera, width, height);
    dirLightLine->Render(pGameCamera,width,height);
    xline->Render(pGameCamera,width, height);
    yline->Render(pGameCamera, width, height);
    zline->Render(pGameCamera,width, height);

   // tline2->Render(-1,0.9,64.0,(char*)(ConvertToString(Cube.GetNumFaces()*10000)+" faces").c_str());
    //tline1->Render(-1,0.5,32.0,(char*)ConvertToString(fps).c_str());
    fLine1->Render(ConvertToString(fps).c_str(),-1.0f,0.9f,30.0f);
    spfaces=Cube.GetNumFaces*100*100+TestMesh.GetNumFaces()
    fLine1->Render(ConvertToString(fps).c_str(),-1.0f,0.0f,45.0f);
    //fLine1->Render(string("ABCDEF"),-1.0f,0.0f,128.0f);
    //fLine1->Render(string("GHIJKLMNOPQRSTUVQXYZ"),-1.0f,-0.3f,64.0f);
    //fLine1->Render(string("abcdefghijkl mnopqrstuvwxzyz,.!/\"':;"),-1.0f,-0.45f,32.0f);
    //fLine1->Render(string(" WV VAW  Such kerning! Ѕла-бла"),-1.0f,-0.55f,32.0f);

}
static int InitScene()
{
    Scale=0;
    lastTime = glfwGetTime();
    frameCount=0;
    try
    {
        pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    catch(const std::bad_alloc&)
    {
        printf("\nCannot allocate camera");
        return -10;
    }
//нормальный шейдер
    char* vertexShaderSorceCode=ReadFile("Shaders/vertexShader.vsh");
    char* fragmentShaderSourceCode=ReadFile("Shaders/fragmentShader.fsh");
    try
    {
        meshShader=new Shader();
    }
    catch(const std::bad_alloc&)
    {
        printf("\nCannot allocate mesh shader");
        return -10;
    }
    meshShader->AddShader((const char*)vertexShaderSorceCode,VertexShader);
    meshShader->AddShader((const char*)fragmentShaderSourceCode,FragmnetShader);
    meshShader->Init();
    shaderProgramID=meshShader->shaderProgramID;
    delete[] vertexShaderSorceCode;
    delete[] fragmentShaderSourceCode;

    gCamViewID =	meshShader->GetUniformLocation("gVC");
    rotateID =	    meshShader->GetUniformLocation("mRotate");
    camPosID =    meshShader->GetUniformLocation("s_vCamPos");

    Cube.Init(meshShader,"Models/sphere2.ho3d");
    TestMesh.Init(meshShader,"Models/torus1.ho3d");
    Plane.Init(meshShader, "Models/plane.ho3d");

    //шейдер тени
    vertexShaderSorceCode=ReadFile("Shaders/vertexShader.vsh");
    fragmentShaderSourceCode=ReadFile("Shaders/fbo.fsh");
    shadowShader=new Shader();
    shadowShader->AddShader((const char*)vertexShaderSorceCode,VertexShader);
    shadowShader->AddShader((const char*)fragmentShaderSourceCode,FragmnetShader);
    shadowShader->Init();

    delete[] vertexShaderSorceCode;
    delete[] fragmentShaderSourceCode;

    //шейдер скайбокса

    vertexShaderSorceCode=ReadFile("Shaders/skybox.vsh");
    fragmentShaderSourceCode=ReadFile("Shaders/skybox.fsh");
    try
    {
        skyboxShader=new Shader();
    }
    catch(const std::bad_alloc&)
    {
        printf("\nCannot allocate skybox shader");
        return -10;
    }
    skyboxShader->AddShader(vertexShaderSorceCode, VertexShader);
    skyboxShader->AddShader(fragmentShaderSourceCode, FragmnetShader);
    skyboxShader->Init();
    shaderProgramID2=skyboxShader->shaderProgramID;
    delete[] vertexShaderSorceCode;
    delete[] fragmentShaderSourceCode;

    //шейдер текста
    vertexShaderSorceCode=ReadFile("Shaders/text2d.vsh");
    fragmentShaderSourceCode=ReadFile("Shaders/text2d.fsh");
    try
    {
        textShader=new Shader();
    }
    catch(const std::bad_alloc&)
    {
        printf("\nCannot allocate text shader");
        return -10;
    }
    textShader->AddShader(vertexShaderSorceCode,VertexShader);
    textShader->AddShader(fragmentShaderSourceCode,FragmnetShader);
    textShader->Init();
    delete[] vertexShaderSorceCode;
    delete[] fragmentShaderSourceCode;
    try
    {
        skybox1= new SkyBox(skyboxShader);
    }
    catch(const std::bad_alloc&)
    {
        printf("\nCannot allocate skybox");
        return -10;
    }

    skybox1->Init("Textures",
                  "sp3right.tga",
                  "sp3left.tga",
                  "sp3top.tga",
                  "sp3bot.tga",
                  "sp3front.tga",
                  "sp3back.tga");
//настройка света и единичных векторов
    dirLightDirID=		meshShader->GetUniformLocation("dLightDir");
    dirLightColID=		meshShader->GetUniformLocation("dLightCol");
    spotLightDirID=		meshShader->GetUniformLocation("sLightDir");
    spotLightColID=		meshShader->GetUniformLocation("sLightCol");
    spotLightCutoffID=	meshShader->GetUniformLocation( "sLightCutoff");
    spotLightPosID=		meshShader->GetUniformLocation("sLightPos");

    pointLightIntID=	meshShader->GetUniformLocation("pLightInt");
    pointLightColID=	meshShader->GetUniformLocation("pLightCol");
    pointLightPosID=	meshShader->GetUniformLocation("pLightPos");

    light1=new DirectionalLight(-1.5f,-1.0f,-1.5f, 0.5f,0.5f,0.5f);
    light2=new PointLight(0,1,-0.2, 1.0,1.0,1.0,0);
    light3=new SpotLight;
    light3->Init(-1.0f,-1.0f,-1.0f,
                 1.0f,1.0f,0.0f,
                 2.0f,2.0f,2.0f,
                 35.0f);
    Vector3f PX(1,0,0);
    Vector3f PY(0,1,0);
    Vector3f PZ(0,0,1);
    Vector3f P0(0,0,0);
    try
    {
        xline= new Line(PX,P0,PX);
        yline= new Line(PY,P0,PY,xline->GetShader());
        zline= new Line(PZ,P0,PZ,xline->GetShader());
        dirLightLine=new Line(P0,light1->GetDir(),light1->GetCol());
        bb1=new Billboard();
    }
    catch(const std::bad_alloc&)
    {
        printf("\nCannot allocate one or more lines");
        return -10;
    }
    bb1->Init("Textures/monster_hellknight.png");
    bb1->SetPos(Vector3f(0,0,0));

    noise1=new PerlinNoise(1,10.3,0.5,2,42);
    try
    {
        tline1=new TextLine2d();
        tline2=new TextLine2d();
        fLine1=new FontLine2d();
    }
    catch(const std::bad_alloc&)
    {
        printf("\nCannot allocate text line");
        return -10;
    }
    tline1->Init(width,height,textShader);
    tline2->Init(width,height,textShader);
    fLine1->Init(string("Fonts/MagistralIC_UTF-8.fnt"),textShader);
    fLine1->SetAspectRatio(width,height);

    //*************Shadow MAP FBO**********/
    smfbo1.Init(width,height);
    //************************************/

    return 0;
}
int main(void)
{
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL
    GLFWwindow* window;
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "HOGL Game", NULL, NULL);
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
