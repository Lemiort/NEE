#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <main.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>

using namespace std;

void CalcFPS() {
    double currentTime = glfwGetTime();
    ++frameCount;
    if (currentTime - lastTime >= 1.0) {
        fps = (double)frameCount / (currentTime - lastTime);
        lastTime += 1.0;
        frameCount = 0;
    }
}

void ErrorCallback(int error, const char* description) {
    fputs(description, stderr);
}
void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_I) spotLight1->target[0] += 0.1;
        if (key == GLFW_KEY_K) spotLight1->target[0] -= 0.1;
        if (key == GLFW_KEY_O) spotLight1->target[1] += 0.1;
        if (key == GLFW_KEY_L) spotLight1->target[1] -= 0.1;
        if (key == GLFW_KEY_U) spotLight1->target[2] += 0.1;
        if (key == GLFW_KEY_J) spotLight1->target[2] -= 0.1;
        if (key == GLFW_KEY_F5) {
            renderType += 1;
            renderType %= 6;
        }
        if (key == GLFW_KEY_PRINT_SCREEN) {
            time_t rawtime;
            struct tm* timeinfo;
            char buffer[80];

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer, 80,
                     "Screenshots/Screenshot %d-%m-%Y %I.%M.%S.tga\0",
                     timeinfo);
            int result = SOIL_save_screenshot(buffer, SOIL_SAVE_TYPE_TGA, 0, 0,
                                              width, height);
            if (result) cout << "\n Screenshot saved as " << buffer;
        } else
            pGameCamera->OnKeyboard(key);
    }
}
void MousePosCallBack(GLFWwindow* window, double x, double y) {
    mouse.Update(x, y);
    if (mouse.rightButtonPressed || true)
        pGameCamera->OnMouse(mouse.posX, mouse.posY);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_2)  // right button
    {
        if (action == GLFW_PRESS) {
            mouse.rightButtonPressed = true;
        } else {
            mouse.rightButtonPressed = false;
        }
    }
}

void FrameBufferSizeCallback(GLFWwindow* window, int w, int h) {
    width = w;
    height = h;
    if (tline1 != NULL) tline1->SetAspectRatio(width, height);
    if (fLine1 != NULL) fLine1->SetAspectRatio(width, height);
    if (smfbo1 != NULL) smfbo1->Init(w, h);
    if (gBuffer1 != NULL) gBuffer1->Init(w, h);
    pGameCamera->OnViewportResize(width, height);
    glViewport(0, 0, width, height);
}

void ShadowPass() {
    smfbo1->BindForWriting();
    // gBuffer1->BindForWriting();
    //���� �������
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // CalcFPS();
    Scale += 0.021f;
    TestMesh.SetRotation(0, 30 * sinf(Scale), 0);
    TestMesh.SetScale(0.02, 0.02, 0.02);
    TestMesh.SetPosition(0, -0.2, 0);
    // light3->SetPos(pGameCamera->GetPos());
    // light3->SetDir(pGameCamera->GetTarget()-pGameCamera->GetPos());
    Camera* lightCam =
        new Camera(width, height, 45, 1, 1000.0f,
                   {spotLight1->GetPos().x, spotLight1->GetPos().y,
                    spotLight1->GetPos().z},
                   glm::vec3(-1.0, -1.0, -1.0), glm::vec3(0.0, 1.0, 0.0));

    Plane.SetScale(30.0f, 30.0f, 30.0f);
    Plane.SetPosition(0.0f, -3.0f, 0.0f);
    Plane.SetRotation(0.0, 0.0, 0.0);
    Assistant TM;  // TM - ��� �������, 2- ��� ������� �������, 3 - ��� �������
                   // ������ ��� ���������
    TM.SetCamera(lightCam->GetPos(), lightCam->GetTarget(), lightCam->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    shadowShader->Use();
    gCamViewID = shadowShader->GetUniformLocation("gVC");
    rotateID = shadowShader->GetUniformLocation("mRotate");
    camPosID = shadowShader->GetUniformLocation("s_vCamPos");

    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    //���������
    {
        //������������ ����
        Assistant LA;
        LA.Scale(directionalLight1->color[0], directionalLight1->color[1],
                 directionalLight1->color[2]);
        glUniformMatrix4fv(dirLightColID, 1, GL_TRUE,
                           (const GLfloat*)LA.GetScaleTrans());
        glUniform3f(dirLightDirID, directionalLight1->direction[0],
                    directionalLight1->direction[1],
                    directionalLight1->direction[2]);
        //��������
        Assistant LA2;
        LA2.Scale(pointLight1->color[0], pointLight1->color[1],
                  pointLight1->color[2]);
        glUniformMatrix4fv(pointLightColID, 1, GL_TRUE,
                           (const GLfloat*)LA2.GetScaleTrans());
        glUniform3f(pointLightPosID, pointLight1->position[0],
                    pointLight1->position[1], pointLight1->position[2]);
        glUniform1f(pointLightIntID, pointLight1->power);
        //���������
        Assistant LA3;
        LA3.Scale(spotLight1->color[0], spotLight1->color[1],
                  spotLight1->color[2]);
        glUniformMatrix4fv(spotLightColID, 1, GL_TRUE,
                           (const GLfloat*)LA3.GetScaleTrans());
        glUniform3f(spotLightDirID, spotLight1->direction[0],
                    spotLight1->direction[1], spotLight1->direction[2]);
        glUniform1f(spotLightCutoffID,
                    cosf((spotLight1->Cutoff) * 3.14 / 180.0f));
        glUniform3f(spotLightPosID, spotLight1->position[0],
                    spotLight1->position[1], spotLight1->position[2]);
    }

    //�������� ������ ��� ���������
    glUniform3f(camPosID, lightCam->GetPos().x, lightCam->GetPos().y,
                lightCam->GetPos().z);

    Cube.SetMaterial(shadowMaterial);
    for (float i = -5.0f; i < 5.0f; i += 0.1f)
        for (float j = -5.0f; j < 5.0f; j += 0.1f) {
            Cube.SetScale(0.05f, 0.05f, 0.05f);
            Cube.SetRotation(0, 30 * sinf(Scale), 0);
            Cube.SetPosition(i, noise1->GetHeight(i, j), j);
            Cube.Render(lightCam);
        }
    Plane.SetMaterial(shadowMaterial);
    Plane.Render(lightCam);

    delete lightCam;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass() {
    //���� �������
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // CalcFPS();
    Scale += 0.021f;
    TestMesh.SetRotation(0, 30 * sinf(Scale), 0);
    TestMesh.SetScale(0.02, 0.02, 0.02);
    TestMesh.SetPosition(0, -0.2, 0);

    Plane.SetScale(30.0f, 30.0f, 30.0f);
    Plane.SetPosition(0.0f, -3.0f, 0.0f);
    Plane.SetRotation(0.0, 0.0, 0.0);
    Assistant TM, TM2;  // TM - ��� �������, 2- ��� ������� �������, 3 - ���
                        // ������� ������ ��� ���������
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(),
                 pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    Camera* lightCam =
        new Camera(width, height, pGameCamera->GetFov(),
                   pGameCamera->GetZNear(), pGameCamera->GetZFar(),
                   {spotLight1->GetPos().x, spotLight1->GetPos().y,
                    spotLight1->GetPos().z},
                   glm::vec3(-1.0, -1.0, -1.0), glm::vec3(0.0, 1.0, 0.0));

    TM2.SetCamera(lightCam->GetPos(), lightCam->GetTarget(), lightCam->GetUp());
    TM2.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    /*meshShader->Use();
    gCamViewID =	meshShader->GetUniformLocation("gVC");
    rotateID =	    meshShader->GetUniformLocation("mRotate");
    camPosID =    meshShader->GetUniformLocation("s_vCamPos");


    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    //���������
    {

        //������������ ����
        Assistant LA;
        LA.Scale(light1->color[0],light1->color[1],light1->color[2]);
        glUniformMatrix4fv(dirLightColID,1, GL_TRUE, (const
    GLfloat*)LA.GetScaleTrans());
        glUniform3f(dirLightDirID,light1->direction[0],light1->direction[1],light1->direction[2]);
        //��������
        Assistant LA2;
        LA2.Scale(light2->color[0],light2->color[1],light2->color[2]);
        glUniformMatrix4fv(pointLightColID,1, GL_TRUE, (const
    GLfloat*)LA2.GetScaleTrans());
        glUniform3f(pointLightPosID,light2->position[0],light2->position[1],light2->position[2]);
        glUniform1f(pointLightIntID,light2->power);
        //���������
        Assistant LA3;
        LA3.Scale(light3->color[0],light3->color[1],light3->color[2]);
        glUniformMatrix4fv(spotLightColID,1, GL_TRUE, (const
    GLfloat*)LA3.GetScaleTrans());
        glUniform3f(spotLightDirID,light3->direction[0],light3->direction[1],light3->direction[2]);
        glUniform1f(spotLightCutoffID,cosf((light3->Cutoff)*3.14/180.0f));
        glUniform3f(spotLightPosID,light3->position[0],light3->position[1],light3->position[2]);
    }

    //�������� ������ ��� ���������
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
    skybox1->Render(pGameCamera);

    /*Texture2D* tempTexture=new Texture2D(gBuffer1->GetTexture(2),false);
    //shadowMeshMaterial->SetShadowTexture(tempTexture);
    mainMaterial->SetColorTexture(tempTexture);
    fLine1->Render(tempTexture->GetParameters(),-1.0f,-0.2f,32.0f);*/
    Plane.SetMaterial(mainMaterial);
    //���� ��������
    {
        meshShader->Use();
        GLuint gLightCamViewID = meshShader->GetUniformLocation("gLightVC");
        gCamViewID = meshShader->GetUniformLocation("gVC");
        rotateID = meshShader->GetUniformLocation("mRotate");
        camPosID = meshShader->GetUniformLocation("s_vCamPos");

        glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
        glUniformMatrix4fv(gLightCamViewID, 1, GL_TRUE,
                           (const GLfloat*)TM2.GetVC());

        //���������
        {
            //������������ ����
            Assistant LA;
            LA.Scale(directionalLight1->color[0], directionalLight1->color[1],
                     directionalLight1->color[2]);
            glUniformMatrix4fv(dirLightColID, 1, GL_TRUE,
                               (const GLfloat*)LA.GetScaleTrans());
            glUniform3f(dirLightDirID, directionalLight1->direction[0],
                        directionalLight1->direction[1],
                        directionalLight1->direction[2]);
            //��������
            Assistant LA2;
            LA2.Scale(pointLight1->color[0], pointLight1->color[1],
                      pointLight1->color[2]);
            glUniformMatrix4fv(pointLightColID, 1, GL_TRUE,
                               (const GLfloat*)LA2.GetScaleTrans());
            glUniform3f(pointLightPosID, pointLight1->position[0],
                        pointLight1->position[1], pointLight1->position[2]);
            glUniform1f(pointLightIntID, pointLight1->power);
            //���������
            Assistant LA3;
            LA3.Scale(spotLight1->color[0], spotLight1->color[1],
                      spotLight1->color[2]);
            glUniformMatrix4fv(spotLightColID, 1, GL_TRUE,
                               (const GLfloat*)LA3.GetScaleTrans());
            glUniform3f(spotLightDirID, spotLight1->direction[0],
                        spotLight1->direction[1], spotLight1->direction[2]);
            glUniform1f(spotLightCutoffID,
                        cosf((spotLight1->Cutoff) * 3.14 / 180.0f));
            glUniform3f(spotLightPosID, spotLight1->position[0],
                        spotLight1->position[1], spotLight1->position[2]);
        }

        //�������� ������ ��� ���������
        glUniform3f(camPosID, pGameCamera->GetPos().x, pGameCamera->GetPos().y,
                    pGameCamera->GetPos().z);
    }
    // Plane.SetMaterial(mainMaterial);
    Plane.Render(pGameCamera);
    Cube.SetMaterial(mainMaterial);
    for (float i = -5.0f; i < 5.0f; i += 0.1f)
        for (float j = -5.0f; j < 5.0f; j += 0.1f) {
            Cube.SetScale(0.05f, 0.05f, 0.05f);
            Cube.SetRotation(0, 30 * sinf(Scale), 0);
            Cube.SetPosition(i, noise1->GetHeight(i, j), j);
            Cube.Render(pGameCamera);
        }
    // delete tempTexture;

    // bb1->Render(pGameCamera);

    /* xline->Render(pGameCamera,width, height);
     yline->Render(pGameCamera, width, height);
     zline->Render(pGameCamera,width, height);

    //
    tline2->Render(-1,0.9,64.0,(char*)(ConvertToString(Cube.GetNumFaces()*10000)+"
    faces").c_str());
     //tline1->Render(-1,0.5,32.0,(char*)ConvertToString(fps).c_str());
     //fLine1->Render(ConvertToString(fps).c_str(),-1.0f,0.9f,30.0f);
     spfaces=Cube.GetNumFaces()*100*100+TestMesh.GetNumFaces()+Plane.GetNumFaces();
     fLine1->Render((ConvertToString(spfaces)+"
    faces").c_str(),-1.0f,0.0f,45.0f);*/
    delete lightCam;
}

void DSBeginLightPasses() {
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // gBuffer1->BindForReading();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DSEndLigtPasses() { glDisable(GL_BLEND); }

void DSLightingPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gBuffer1->BindForReading();

    // GLsizei HalfWidth = (GLsizei)(width / 2.0f);
    // GLsizei HalfHeight = (GLsizei)(height / 2.0f);

    // gBuffer1->CheckTextures();
    /*
    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
    glBlitFramebuffer(0, 0,width, height, 0, 0, HalfWidth, HalfHeight,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, width, height, 0, HalfHeight, HalfWidth, height,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, width, height, HalfWidth, HalfHeight, width, height,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
    glBlitFramebuffer(0, 0, width, height, HalfWidth, 0, width, HalfHeight,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);*/
    switch (renderType) {
        case 1:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        case 2:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        case 3:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        case 4:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
    }

    /* fLine1->Render(ConvertToString(fps).c_str(),-1.0f,0.9f,30.0f);
     spfaces=Cube.GetNumFaces()*100*100+TestMesh.GetNumFaces()+Plane.GetNumFaces();
     fLine1->Render((ConvertToString(spfaces)+"
     faces").c_str(),-1.0f,0.0f,45.0f);*/
}

void DSStencilPass(Light& light) {
    // m_nullTech.Enable();
    //�������� ������
    DSStencilPassShader->Use();

    // ��������� ������ ����� / ������� � �������� ��������
    gBuffer1->BindForStencilPass();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  //??

    glDisable(GL_CULL_FACE);

    glClear(GL_STENCIL_BUFFER_BIT);

    // ��� ����� ���� ���������, �� �� �����, ��� �� �� ������
    // ������� ��������. ����� ������ ���� �������.
    glStencilFunc(GL_ALWAYS, 0, 0);

    // glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
    // glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR,
                        GL_KEEP);  // TODO ������� ��������
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR,
                        GL_KEEP);  // TODO ������� ��������

    Assistant TM;  // TM - ��� �������, 2- ��� ������� �������, 3 - ��� �������
                   // ������ ��� ���������
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(),
                 pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    //���������� ����� ���������� ������
    gCamViewID = DSStencilPassShader->GetUniformLocation("gVC");
    rotateID = DSStencilPassShader->GetUniformLocation("mRotate");
    camPosID = DSStencilPassShader->GetUniformLocation("s_vCamPos");

    //��������� ������� ������
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());

    light.SetMaterial(DSStencilPassMaterial);
    light.Render(pGameCamera);
}

void DSPointLightPass(PointLight& pointLight) {
    // gBuffer1->BindForReading();
    gBuffer1->BindForLightPass();

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    Assistant TM;  // TM - ��� �������, 2- ��� ������� �������, 3 - ��� �������
                   // ������ ��� ���������
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(),
                 pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    //�������� ������
    DSPointLightShader->Use();

    //���������� ����� ���������� ������
    gCamViewID = DSPointLightShader->GetUniformLocation("gVC");
    rotateID = DSPointLightShader->GetUniformLocation("mRotate");
    camPosID = DSPointLightShader->GetUniformLocation("s_vCamPos");

    //��������� ������� ������
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    //���������� �������� ������ ��� ���������
    glUniform3f(camPosID, pGameCamera->GetPos().x, pGameCamera->GetPos().y,
                pGameCamera->GetPos().z);

    //��������� �������� � ������
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(0), 4);  // world pos
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(1), 5);  // diffuse
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(2), 6);  // normal
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(3), 7);  // UV
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(4), 8);  // specular

    //�������� ������ �� �������
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glClear(/*GL_COLOR_BUFFER_BIT |*/GL_DEPTH_BUFFER_BIT);

    //���������� ����� ���������� �����
    pointLightPosID =
        DSPointLightShader->GetUniformLocation("s_vPointLightPos");
    pointLightIntID =
        DSPointLightShader->GetUniformLocation("pointLightIntensity");
    pointLightColID = DSPointLightShader->GetUniformLocation("pointLightColor");

    //��������� ��������� ����� ��� ��������� 1
    Assistant LA2;
    LA2.Scale(pointLight.color[0], pointLight.color[1], pointLight.color[2]);
    glUniformMatrix4fv(pointLightColID, 1, GL_TRUE,
                       (const GLfloat*)LA2.GetScaleTrans());
    glUniform3f(pointLightPosID, pointLight.position[0], pointLight.position[1],
                pointLight.position[2]);
    glUniform1f(pointLightIntID, pointLight.power);

    pointLight.SetMaterial(DSPointLightMaterial);
    pointLight.Render(pGameCamera);

    glCullFace(GL_FRONT);
    glDisable(GL_BLEND);
}

void DSSpotLightPass(SpotLight& spotLight) {
    // gBuffer1->BindForReading();
    gBuffer1->BindForLightPass();
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    Assistant TM;  // TM - ��� �������, 2- ��� ������� �������, 3 - ��� �������
                   // ������ ��� ���������
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(),
                 pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    //�������� ������
    DSSpotLightShader->Use();

    //���������� ����� ���������� ������
    gCamViewID = DSSpotLightShader->GetUniformLocation("gVC");
    rotateID = DSSpotLightShader->GetUniformLocation("mRotate");
    camPosID = DSSpotLightShader->GetUniformLocation("s_vCamPos");

    //��������� ������� ������
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    //���������� �������� ������ ��� ���������
    glUniform3f(camPosID, pGameCamera->GetPos().x, pGameCamera->GetPos().y,
                pGameCamera->GetPos().z);

    //��������� �������� � ������
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(0), 4);  // world pos
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(1), 5);  // diffuse
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(2), 6);  // normal
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(3), 7);  // UV
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(4), 8);  // specular

    //���������� ����� ���������� �����
    spotLightPosID = DSSpotLightShader->GetUniformLocation("sLightPos");
    spotLightColID = DSSpotLightShader->GetUniformLocation("sLightCol");
    spotLightDirID = DSSpotLightShader->GetUniformLocation("sLightDir");
    spotLightCutoffID = DSSpotLightShader->GetUniformLocation("sLightCutoff");

    //��������� ��������� ����� ��� ��������� 1
    Assistant LA2;
    LA2.Scale(spotLight.color[0], spotLight.color[1], spotLight.color[2]);
    glUniform3f(spotLightPosID, spotLight.position[0], spotLight.position[1],
                spotLight.position[2]);
    glUniformMatrix4fv(spotLightColID, 1, GL_TRUE,
                       (const GLfloat*)LA2.GetScaleTrans());
    glUniform3f(spotLightDirID, spotLight.direction[0], spotLight.direction[1],
                spotLight.direction[2]);
    glUniform1f(spotLightCutoffID, cosf(ToRadian(spotLight.Cutoff)));

    //�������� ������ �� �������
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    spotLight.SetMaterial(DSSpotLightMaterial);
    spotLight.Render(pGameCamera);

    glCullFace(GL_FRONT);
    glDisable(GL_BLEND);
}

void DSDirectionalLightPass(DirectionalLight& directionalLight) {
    // gBuffer1->BindForReading();
    gBuffer1->BindForLightPass();
    // glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    Assistant TM;  // TM - ��� �������, 2- ��� ������� �������, 3 - ��� �������
                   // ������ ��� ���������
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(),
                 pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    DSDirectionalLightShader->Use();
    //�������� ����� ���������� ������
    gCamViewID = DSDirectionalLightShader->GetUniformLocation("gVC");
    rotateID = DSDirectionalLightShader->GetUniformLocation("mRotate");
    camPosID = DSDirectionalLightShader->GetUniformLocation("s_vCamPos");
    //��������� �������� ������ ��� ���������
    glUniform3f(camPosID, pGameCamera->GetPos().x, pGameCamera->GetPos().y,
                pGameCamera->GetPos().z);
    //��������� ������� ������
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());

    //�������� ����� ���������� �����
    dirLightColID =
        DSDirectionalLightShader->GetUniformLocation("dirLightColor");
    dirLightDirID =
        DSDirectionalLightShader->GetUniformLocation("dirLightDirection");

    //�������� ���������� �����
    Assistant LA2;
    LA2.Scale(directionalLight.color[0], directionalLight.color[1],
              directionalLight.color[2]);
    glUniformMatrix4fv(dirLightColID, 1, GL_TRUE,
                       (const GLfloat*)LA2.GetScaleTrans());
    glUniform3f(dirLightDirID, directionalLight.direction[0],
                directionalLight.direction[1], directionalLight.direction[2]);

    //��������� �������� � ������
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(0),
                                           4);  // world pos
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(1),
                                           5);  // diffuse
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(2),
                                           6);  // normal
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(3), 7);  // UV
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(4),
                                           8);  // specular

    directionalLight.SetMaterial(DSDirectionalLightMaterial);

    /*printf("\n %f %f %f",directionalLight1->direction[0],
                            directionalLight1->direction[1],
                            directionalLight1->direction[2]);*/
    // glUniform1f(pointLightIntID,pointLight1->power);

    //�������� ������ �� �������
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);

    directionalLight.Render(pGameCamera);
    // glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void DSFinalPass() {
    gBuffer1->BindForFinalPass();
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH,
                      WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void InterfacePass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glDisable(GL_DEPTH_TEST);
    xline->Render(pGameCamera);
    yline->Render(pGameCamera);
    zline->Render(pGameCamera);
    std::string strCampos = ConvertToString(pGameCamera->GetPos().x) + "; " +
                            ConvertToString(pGameCamera->GetPos().y) + "; " +
                            ConvertToString(pGameCamera->GetPos().z);
    // fLine1->Render((strCampos).c_str(),-1.0f,0.0f,24.0f);
    fLine1->SetText((strCampos).c_str());
    fLine1->SetPosition(-1.0f, 0.0f, 24.0f);
    fLine1->Render(pGameCamera);
    // gBuffer1->CheckTextures();
    CalcFPS();
    // fLine1->Render(ConvertToString(fps),-1.0f,0.9f,24.0f);
    fLine1->SetText(ConvertToString(fps));
    fLine1->SetPosition(-1.0f, 0.9f, 24.0f);
    fLine1->Render(pGameCamera);

    if (renderType == 0) {
        // fLine1->Render("Deferred shading",-1.0f,-0.2f,36.0f);
        fLine1->SetText("Deferred shading");
        fLine1->SetPosition(-1.0f, -0.2f, 36.0f);
        fLine1->Render(pGameCamera);
    }

    // pointLight1->Render(30,width, height, 1, 1000,pGameCamera);
    dirLightLine->Render(pGameCamera);
    // glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void DSGeometryPass() {
    gBuffer1->BindForGeomPass();
    // gBuffer1->BindForWriting();
    //������ �������������� ������ ��������� ���� �������
    glDepthMask(GL_TRUE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    CalcFPS();

    Assistant TM;  // TM - ��� �������, 2- ��� ������� �������, 3 - ��� �������
                   // ������ ��� ���������
    TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(),
                 pGameCamera->GetUp());
    TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

    DSGeometryPassShader->Use();
    gCamViewID = DSGeometryPassShader->GetUniformLocation("gVC");

    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());

    Cube.SetMaterial(DSGeometryPassMaterial);
    for (float i = -5.0f; i < 5.0f; i += 0.1f)
        for (float j = -5.0f; j < 5.0f; j += 0.1f) {
            Cube.SetScale(0.05f, 0.05f, 0.05f);
            Cube.SetRotation(0, 30 * sinf(Scale), 0);
            Cube.SetPosition(i, noise1->GetHeight(i, j), j);
            Cube.Render(pGameCamera);
        }

    // glBindFramebuffer(GL_FRAMEBUFFER,0);

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}

void RenderScene(GLFWwindow* window) {
    // ShadowPass();
    // RenderPass();
    // DSGeometryPass();
    // DSBeginLightPasses();//������ ���� �� �����
    // DSPointLightPass();
    // DSLightingPass();
    //������� deffered shading
    if (renderType == 0) {
        gBuffer1->StartFrame();
        DSGeometryPass();
        // ��� ����, ��� �� ��������� ����� ��������� ����� ��� ������������,
        // ��� �� �� ����������� � � ������� �����, ��� ��� ���� ����������
        // ������ ��� �������� ������� ���������.
        glEnable(GL_STENCIL_TEST);
        // DSBeginLightPasses();//��� ��������
        {
            DSStencilPass(*pointLight1);
            DSPointLightPass(*pointLight1);

            DSStencilPass(*pointLight2);
            DSPointLightPass(*pointLight2);

            DSStencilPass(*spotLight1);
            DSSpotLightPass(*spotLight1);
        }
        // ������������� ����� �� ��������� ��������
        // ��� ��� ��� �������� �� ���������� �����������.
        glDisable(GL_STENCIL_TEST);

        DSDirectionalLightPass(*directionalLight1);

        DSFinalPass();
        // DSEndLigtPasses();
    }
    //������������ ���
    else if (renderType <= 4) {
        DSGeometryPass();
        DSLightingPass();
    } else {
        RenderPass();
    }
    InterfacePass();
}
void PreInitScene(GLFWwindow* window) {
    Scale = 0;
    lastTime = glfwGetTime();
    frameCount = 0;

    //������ ������
    {
        char* vertexShaderSorceCode = ReadFile("shaders/text2d.vsh");
        char* fragmentShaderSourceCode = ReadFile("shaders/text2d.fsh");

        textShader = make_shared<Shader>();
        textShader->Add(Shader::Type::kVertex, vertexShaderSorceCode);
        textShader->Add(Shader::Type::kFragmnet, fragmentShaderSourceCode);
        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;
    }
    fLine1 = new FontLine2d();
    fLine1->Init(string("Fonts/MagistralIC_UTF-8.fnt"), textShader);
    fLine1->SetAspectRatio(width, height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // fLine1->Render("Loading...",-1.0f,-0.1f,72.0f);
    fLine1->SetText("Loading...");
    fLine1->SetPosition(-1.0f, -0.1f, 72.0f);
    fLine1->Render(pGameCamera);
    glfwSwapBuffers(window);
    initialized = false;
}

void InitRender(GLFWwindow* window, string message) {
    // thread thr(InitScene,hiddenWindow);

    // thr.detach();
    // InitScene(window);

    // while(!glfwWindowShouldClose(window) && initialized == false)
    if (initialized == false) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CalcFPS();
        // fLine1->Render(ConvertToString(fps),-1.0f,0.9f,24.0f);
        fLine1->SetText(ConvertToString(fps));
        fLine1->SetPosition(-1.0f, 0.9f, 24.0f);
        fLine1->Render(pGameCamera);

        // fLine1->Render(message,-1.0f,-0.1f,36.0f);
        fLine1->SetText(message);
        fLine1->SetPosition(-1.0f, -0.1f, 36.0f);
        fLine1->Render(pGameCamera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // thr.join();
}

int InitScene(GLFWwindow* window) {
    /*std::thread thr(InitRender, window, string("test"));

    thr.detach();*/

    initialized = false;
    glfwMakeContextCurrent(window);
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    } else {
        printf("\nGLEW status is %d \n", res);
    }
    /*Scale=0;
    lastTime = glfwGetTime();
    frameCount=0;

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);**/

    //���������� ������
    // InitRender(window, "Normal shader loading...");
    char* vertexShaderSorceCode = ReadFile("shaders/vertexShader.vsh");
    char* fragmentShaderSourceCode = ReadFile("shaders/fragmentShader.fsh");
    {
        meshShader = make_shared<Shader>();
        meshShader->Add(Shader::Type::kVertex,
                        (const char*)vertexShaderSorceCode);
        meshShader->Add(Shader::Type::kFragmnet,
                        (const char*)fragmentShaderSourceCode);
        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;

        gCamViewID = meshShader->GetUniformLocation("gVC");
        rotateID = meshShader->GetUniformLocation("mRotate");
        camPosID = meshShader->GetUniformLocation("s_vCamPos");
    }

    //�������� ��������
    {
        InitRender(window, "Main material loading...");
        mainMaterial = make_shared<Material>();
        mainMaterial->Init(meshShader);
    }

    // Cube.Init(mainMaterial,"models/sphere2.ho3d");
    TestMesh.Init(mainMaterial, "models/torus1.ho3d");
    // Plane.Init(mainMaterial, "models/plane.ho3d");

    //������ ����
    {
        InitRender(window, "Shade shader loading...");
        vertexShaderSorceCode = ReadFile("shaders/fbo.vsh");
        fragmentShaderSourceCode = ReadFile("shaders/fbo.fsh");
        shadowShader = make_shared<Shader>();
        shadowShader->Add(Shader::Type::kVertex,
                          (const char*)vertexShaderSorceCode);
        shadowShader->Add(Shader::Type::kFragmnet,
                          (const char*)fragmentShaderSourceCode);

        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;
    }

    //�������� ����
    {
        InitRender(window, "Shade Material loading...");
        shadowMaterial = make_shared<Material>();
        shadowMaterial->Init(shadowShader);
    }

    //��������� ������
    {
        InitRender(window, "Shaded shader loading...");
        vertexShaderSorceCode = ReadFile("shaders/shadowed.vsh");
        // fragmentShaderSourceCode=ReadFile("shaders/fragmentShader.fsh");
        fragmentShaderSourceCode = ReadFile("shaders/shadowed.fsh");
        shadowMeshShader = make_shared<Shader>();
        shadowMeshShader->Add(Shader::Type::kVertex,
                              (const char*)vertexShaderSorceCode);
        shadowMeshShader->Add(Shader::Type::kFragmnet,
                              (const char*)fragmentShaderSourceCode);

        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;
    }

    //��������� ��������
    {
        InitRender(window, "Shaded material loading...");
        shadowMeshMaterial = make_shared<Material>();
        shadowMeshMaterial->Init(shadowMeshShader);
        whiteTexture = make_shared<Texture2D>();
        whiteTexture->Load("textures/white.png");
        shadowMeshMaterial->SetColorTexture(whiteTexture);
        Cube.Init(shadowMeshMaterial, "models/normal_geosphere.ho3d");
        Plane.Init(shadowMeshMaterial, "models/normal_plane.ho3d");
    }

    //������ StencilPass ��������
    {
        InitRender(window, "StencilPass shader loading...");
        //������ ������
        {
            vertexShaderSorceCode = ReadFile("shaders/DSStencilPass.vsh");
            DSStencilPassShader = make_shared<Shader>();
            DSStencilPassShader->Add(Shader::Type::kVertex,
                                     (const char*)vertexShaderSorceCode);

            delete[] vertexShaderSorceCode;
        }
        //������ ��������
        {
            DSStencilPassMaterial = make_shared<Material>();
            DSStencilPassMaterial->Init(DSStencilPassShader);
        }
    }

    //������ GBuffer� � ��� ������, ��������
    {
        InitRender(window, "Gbuffer shader loading...");
        gBuffer1 = new GBuffer;
        gBuffer1->Init(width, height);
        //������ ������
        {
            vertexShaderSorceCode = ReadFile("shaders/DSGeometryPass.vsh");
            fragmentShaderSourceCode = ReadFile("shaders/DSGeometryPass.fsh");
            DSGeometryPassShader = make_shared<Shader>();
            DSGeometryPassShader->Add(Shader::Type::kVertex,
                                      (const char*)vertexShaderSorceCode);
            DSGeometryPassShader->Add(Shader::Type::kFragmnet,
                                      (const char*)fragmentShaderSourceCode);

            delete[] vertexShaderSorceCode;
            delete[] fragmentShaderSourceCode;
        }
        //������ ��������
        {
            DSGeometryPassMaterial = make_shared<Material>();
            DSGeometryPassMaterial->Init(DSGeometryPassShader);
        }
    }

    //������� �����
    {
        InitRender(window, "Light shader loading...");
        //������ ��������� �����
        {//������ ������
         {vertexShaderSorceCode = ReadFile("shaders/DSPointLight.vsh");
        fragmentShaderSourceCode = ReadFile("shaders/DSPointLight.fsh");
        DSPointLightShader = make_shared<Shader>();
        DSPointLightShader->Add(Shader::Type::kVertex,
                                (const char*)vertexShaderSorceCode);
        DSPointLightShader->Add(Shader::Type::kFragmnet,
                                (const char*)fragmentShaderSourceCode);

        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;
    }
    //������ ��������
    {
        DSPointLightMaterial = make_shared<Material>();
        DSPointLightMaterial->Init(DSPointLightShader);
    }
}

//������ ������������� �����
{//������ ������
 {vertexShaderSorceCode = ReadFile("shaders/DSDirectionalLight.vsh");
fragmentShaderSourceCode = ReadFile("shaders/DSDirectionalLight.fsh");
DSDirectionalLightShader = make_shared<Shader>();
DSDirectionalLightShader->Add(Shader::Type::kVertex,
                              (const char*)vertexShaderSorceCode);
DSDirectionalLightShader->Add(Shader::Type::kFragmnet,
                              (const char*)fragmentShaderSourceCode);

delete[] vertexShaderSorceCode;
delete[] fragmentShaderSourceCode;
}
//������ ��������
{
    DSDirectionalLightMaterial = make_shared<Material>();
    DSDirectionalLightMaterial->Init(DSDirectionalLightShader);
}
}

//������ ���������� �����
{
    //������ ������
    {
        vertexShaderSorceCode = ReadFile("shaders/DSSpotLight.vsh");
        fragmentShaderSourceCode = ReadFile("shaders/DSSpotLight.fsh");
        DSSpotLightShader = make_shared<Shader>();
        DSSpotLightShader->Add(Shader::Type::kVertex,
                               (const char*)vertexShaderSorceCode);
        DSSpotLightShader->Add(Shader::Type::kFragmnet,
                               (const char*)fragmentShaderSourceCode);

        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;
    }
    //������ ��������
    {
        DSSpotLightMaterial = make_shared<Material>();
        DSSpotLightMaterial->Init(DSSpotLightShader);
    }
}
}

//������ ���������
{
    InitRender(window, "Skybox shader loading...");
    vertexShaderSorceCode = ReadFile("shaders/skybox.vsh");
    fragmentShaderSourceCode = ReadFile("shaders/skybox.fsh");

    skyboxShader = make_shared<Shader>();

    skyboxShader->Add(Shader::Type::kVertex, vertexShaderSorceCode);
    skyboxShader->Add(Shader::Type::kFragmnet, fragmentShaderSourceCode);
    delete[] vertexShaderSorceCode;
    delete[] fragmentShaderSourceCode;
}

/*//������ ������
{
vertexShaderSorceCode=ReadFile("shaders/text2d.vsh");
fragmentShaderSourceCode=ReadFile("shaders/text2d.fsh");


textShader=new Shader();
textShader->AddShader(vertexShaderSorceCode,VertexShader);
textShader->AddShader(fragmentShaderSourceCode,FragmnetShader);
textShader->Init();
delete[] vertexShaderSorceCode;
delete[] fragmentShaderSourceCode;
}*/

skybox1 = new SkyBox(skyboxShader);
skybox1->Init("textures", "sp3right.tga", "sp3left.tga", "sp3top.tga",
              "sp3bot.tga", "sp3front.tga", "sp3back.tga");
//��������� ����� � ��������� ��������
{
    InitRender(window, "Init lights...");
    dirLightDirID = meshShader->GetUniformLocation("dLightDir");
    dirLightColID = meshShader->GetUniformLocation("dLightCol");
    spotLightDirID = meshShader->GetUniformLocation("sLightDir");
    spotLightColID = meshShader->GetUniformLocation("sLightCol");
    spotLightCutoffID = meshShader->GetUniformLocation("sLightCutoff");
    spotLightPosID = meshShader->GetUniformLocation("sLightPos");

    pointLightIntID = meshShader->GetUniformLocation("pLightInt");
    pointLightColID = meshShader->GetUniformLocation("pLightCol");
    pointLightPosID = meshShader->GetUniformLocation("pLightPos");

    directionalLight1 = new DirectionalLight(-1.5f, -1.0f, -1.5f,  // direction
                                             0.5f, 0.5f, 0.5f,     // color
                                             DSDirectionalLightMaterial);
    pointLight1 = new PointLight(0, 1, -0.2,     // position
                                 1.0, 1.0, 1.0,  // color
                                 1.0, DSPointLightMaterial);
    pointLight2 = new PointLight(1.5, 0.4, 0.0,  // position
                                 0.3, 0.3, 1.0,  // color
                                 1.1,            // power
                                 DSPointLightMaterial);
    spotLight1 = new SpotLight(1.5f, 0.0f, 0.5f,  // target
                               1.0f, 0.4f, 0.4f,  // color
                               0.0f, 1.0f, 0.0f,  // position
                               35.0f,             // cutoff in degrees
                               DSSpotLightMaterial);
    Vector3f PX(1, 0, 0);
    Vector3f PY(0, 1, 0);
    Vector3f PZ(0, 0, 1);
    Vector3f P0(0, 0, 0);

    xline = new Line(PX, P0, PX);
    yline = new Line(PY, P0, PY, xline->GetShader());
    zline = new Line(PZ, P0, PZ, xline->GetShader());
    dirLightLine =
        new Line(P0, directionalLight1->GetDir(), directionalLight1->GetCol());
}

//������
{
    InitRender(window, "Final steps...");
    // bb1 = new Billboard();
    // bb1->Init("textures/monster_hellknight.png");
    // bb1->SetPos(Vector3f(0, 0, 0));

    noise1 = new PerlinNoise(1, 10.3, 0.5, 2, 42);

    tline1 = new TextLine2d();
    tline2 = new TextLine2d();
    tline1->Init(width, height, textShader);
    tline2->Init(width, height, textShader);
    // fLine1->Init(string("Fonts/MagistralIC_UTF-8.fnt"),textShader);
    // fLine1->SetAspectRatio(width,height);

    //*************Shadow MAP FBO**********/
    // smfbo1 = new ShadowMapFBO();
    // smfbo1->Init(width,height);
    //************************************/
    // pGameCamera = new
    // Camera(width,height,light3->GetPos(),Vector3f(-1.0,-1.0,-1.0),Vector3f(0.0,1.0,0.0));
}
glFlush();
initialized = true;
return 0;
}

int main(int argc, char** argv) {
    renderType = 0;
    glfwWindowHint(GLFW_SAMPLES, 4);                // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);  // We don't want the old OpenGL
    GLFWwindow* window;
    glfwSetErrorCallback(reinterpret_cast<GLFWerrorfun>(&ErrorCallback));
    // glewExperimental = true; // Needed for core profile
    if (!glfwInit()) exit(EXIT_FAILURE);

    //���������
    string title("HOGL ");
    // title+= AutoVersion::STATUS;
    // title+=" ";
    // title+=AutoVersion::FULLVERSION_STRING;
    // title+=" build ";
    // title+=ConvertToString(AutoVersion::BUILDS_COUNT);

    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(),
                              /*glfwGetPrimaryMonitor()*/ nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    hiddenWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(),
                                    nullptr, window);

    glfwSetFramebufferSizeCallback(
        window,
        reinterpret_cast<GLFWframebuffersizefun>(&FrameBufferSizeCallback));
    width = WINDOW_WIDTH;
    height = WINDOW_HEIGHT;
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, reinterpret_cast<GLFWkeyfun>(&KeyCallback));
    glfwSetCursorPosCallback(
        window, reinterpret_cast<GLFWcursorposfun>(&MousePosCallBack));
    glfwSetMouseButtonCallback(
        window, reinterpret_cast<GLFWmousebuttonfun>(&MouseButtonCallback));
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    } else {
        printf("\nGLEW status is %d \n", res);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //�������� ������������
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    PreInitScene(window);
    // InitRender(window,"test");
    if (InitScene(window) != 0) return -1;

    while (!glfwWindowShouldClose(window)) {
        RenderScene(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwDestroyWindow(hiddenWindow);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
