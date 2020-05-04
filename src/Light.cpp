#include "Light.h"
#include <iomanip>

void DirectionalLight::SetCol(glm::vec3 col) {
    color[0] = col.x;
    color[1] = col.y;
    color[2] = col.z;
}

void DirectionalLight::SetDir(glm::vec3 dir) {
    direction[0] = dir.x;
    direction[1] = dir.y;
    direction[2] = dir.z;
}

DirectionalLight::DirectionalLight(GLfloat d1, GLfloat d2, GLfloat d3,
                                   GLfloat r, GLfloat g, GLfloat b,
                                   shared_ptr<Material> _mat) {
    direction[0] = d1;
    direction[1] = d2;
    direction[2] = d3;
    direction[3] = 1;
    color[0] = r;
    color[1] = g;
    color[2] = b;

    mesh = make_shared<Mesh>();
    // mesh->Init(_mat,"models/quad2x2front.ho3d");
    mesh->Init(_mat, "models/cube2x2x2.ho3d");
}

void DirectionalLight::Render(Camera* cam) {
    // mesh->SetScale(1.0,1.0,1.0);
    mesh->SetPosition(0, 0, 0);
    mesh->Render(cam);
}

glm::vec3 DirectionalLight::GetDir() {
    return glm::vec3(direction[0], direction[1], direction[2]);
}
glm::vec3 DirectionalLight::GetCol() {
    return glm::vec3(color[0], color[1], color[2]);
}

DirectionalLight::DirectionalLight() {}
DirectionalLight::~DirectionalLight() {}

glm::vec3 SpotLight::GetPos() {
    return glm::vec3(position[0], position[1], position[2]);
}

SpotLight::SpotLight() {}

SpotLight::SpotLight(GLfloat t1, GLfloat t2, GLfloat t3, GLfloat r, GLfloat g,
                     GLfloat b, float p1, float p2, float p3, float cut,
                     shared_ptr<Material> _mat) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
    position[0] = p1;
    position[1] = p2;
    position[2] = p3;
    target[0] = t1;
    target[1] = t2;
    target[2] = t3;
    direction[0] = t1 - p1;
    direction[1] = t2 - p2;
    direction[2] = t3 - p3;
    direction[3] = 1;
    Cutoff = cut;

    mesh = make_shared<Mesh>();
    mesh->Init(_mat, "models/cone2.ho3d");
    mesh->SetRotation(90, 0, 0);
}

void SpotLight::Render(Camera* cam) {

    //======Предыдущее решение задачи========
    direction[0] = target[0] - position[0];
    direction[1] = target[1] - position[1];
    direction[2] = target[2] - position[2];
    direction[3] = 1;

    glm::vec3 dir(direction[0], direction[1], direction[2]);
    glm::vec3 proj(0, 1, 0);
    glm::vec3 directionXY(direction[0], direction[1], 0);
    glm::vec3 directionYZ(0, direction[1], direction[2]);
    glm::vec3 directionXZ(direction[0], 0, direction[2]);

    float xrot = 0, yrot = 0, zrot = 0;

    cout << "\rproj:(" << setprecision(1);

    // X axis
    {
        // get the pojection
        glm::vec3 projYZ(0, proj.y, proj.z);

        // calc rotation
        if (glm::cross(projYZ,directionYZ).x > 0)
            xrot = -glm::degrees(glm::angle(directionYZ, projYZ));
        else
            xrot = glm::degrees(glm::angle(directionYZ, projYZ));

        // update up vector
        proj = glm::rotate(proj,-xrot, glm::vec3(1, 0, 0));
        cout << proj.x << ",  " << proj.y << ",  " << proj.z << ");  (";
    }

    // Y axis
    if (proj != dir) {
        // get the pojection
        glm::vec3 projXZ(proj.x, 0, proj.z);

        // calc the rotation
        if (glm::cross(projXZ,directionXZ).y > 0)
            yrot = -glm::degrees(glm::angle(directionXZ, projXZ));
        else
            yrot = glm::degrees(glm::angle(directionXZ, projXZ));

        // update the up vector
        proj = glm::rotate(proj,-yrot, glm::vec3(0, 1, 0));
        cout << proj.x << ",  " << proj.y << ",  " << proj.z << ");  (";

        // Z axis
        if (proj != dir) {
            // get the pojection
            glm::vec3 projXY(proj.x, proj.y, 0);

            // calc the rotation
            if (glm::cross(projXY,directionXY).z > 0)
                zrot = glm::degrees(glm::angle(directionXY, projXY));
            else
                zrot = -glm::degrees(glm::angle(directionXY, projXY));
            proj = glm::rotate(proj,-zrot, glm::vec3(0, 0, 1));
        }
    }
    cout << proj.x << ",  " << proj.y << ",  " << proj.z << ");  rot: ";

    cout << setprecision(1) << (int)xrot << " " << (int)yrot << " " << (int)zrot
         << ", dir:(" << direction[0] << ", " << direction[1] << ","
         << direction[2] << ")";

    mesh->SetRotation(xrot, yrot, zrot);
    //=======================================================

    /*//==========решение на основе сферических координат=======
    glm::vec3 sphericalDirection = ToSphericalCoordinates( dir );
    glm::vec3 sphericalUp = ToSphericalCoordinates( glm::vec3(0.0,
                                                            1.0,
                                                            0.0) );
    mesh->SetRotate( sphericalDirection.y-sphericalUp.y,
                    0.0f,
                    sphericalDirection.z - sphericalUp.z);
    //=========================================================*/

    //=====решение на основе поворота вокруг вектора============
    // glm::vec3 rotateVector =  dir.Cross(proj);
    /*mesh->SetVectorRotate(rotateVector,
                          glm::angle(dir,proj));*/
    // mesh->SetVectorRotate(proj,
    //                       -90.0f);
    //==========================================================

    // TODO идея: использовать скалярное произведение векторов

    mesh->SetScale(glm::length(dir) * cos(glm::radians(Cutoff)), glm::length(dir),
                   glm::length(dir) * cos(glm::radians(Cutoff)));

    mesh->SetPosition(position[0], position[1], position[2]);
    mesh->Render(cam);
}

void SpotLight::SetTarget(glm::vec3 _target) {
    target[0] = _target.x;
    target[1] = _target.y;
    target[2] = _target.z;
    direction[0] = target[0] - position[0];
    direction[1] = target[1] - position[1];
    direction[2] = target[2] - position[2];
    direction[3] = 1;
}

SpotLight::~SpotLight() {}
void SpotLight::Init(GLfloat d1, GLfloat d2, GLfloat d3, GLfloat r, GLfloat g,
                     GLfloat b, float p1, float p2, float p3, float cut) {
    direction[0] = d1;
    direction[1] = d2;
    direction[2] = d3;
    direction[3] = 1;
    color[0] = r;
    color[1] = g;
    color[2] = b;
    position[0] = p1;
    position[1] = p2;
    position[2] = p3;
    Cutoff = cut;
}

void SpotLight::SetPos(glm::vec3 pos) {
    position[0] = pos.x;
    position[1] = pos.y;
    position[2] = pos.z;

    direction[0] = target[0] - position[0];
    direction[1] = target[1] - position[1];
    direction[2] = target[2] - position[2];
    direction[3] = 1;
}

PointLight::PointLight(float d1, float d2, float d3, float r, float g, float b,
                       float p, shared_ptr<Material> _mat) {
    position[0] = d1;
    position[1] = d2;
    position[2] = d3;
    color[0] = r;
    color[1] = g;
    color[2] = b;
    power = p;
    temp[0] = 0;
    temp[1] = 0;
    temp[2] = 0;
    /*char* vertexShaderSorceCode=ReadFile("shaders/lightVS.vsh");
    char* fragmentShaderSourceCode=ReadFile("shaders/lightFS.fsh");
    shaderProgram=new Shader();
    shaderProgram->AddShader(vertexShaderSorceCode,VertexShader);
    shaderProgram->AddShader(fragmentShaderSourceCode,FragmnetShader);
    shaderProgram->Init();

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(temp),NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(temp),temp);
    positionID=	    shaderProgram->GetAttribLocation("position");
    gWorldID=	    shaderProgram->GetUniformLocation("gWVCP");
    PixelColorID=   shaderProgram->GetUniformLocation("PixelColor");
    PointSizeID=    shaderProgram->GetUniformLocation("size");*/

    sphere = make_shared<Mesh>();
    sphere->Init(_mat, "models/normal_geosphere.ho3d");
    radius = CalcSphereSize() / 2;
    std::cout << "\nLight radius is " << radius;
}
PointLight::~PointLight() {}
void PointLight::Render(Camera* cam) {
    /*Assistant TM;
    TM.WorldPos(position[0],position[1],position[2]);
    TM.SetCamera(cam->GetPos(), cam->GetTarget(), cam->GetUp());
    TM.SetPerspectiveProj(cam, Width, Height, zNear, zFar);*/

    /*shaderProgram->Use();
                    //glUseProgram(shaderProgramID);
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    glVertexAttribPointer(positionID,3,GL_FLOAT,GL_FALSE,0,0);


                    glUniformMatrix4fv(gWorldID, 1, GL_TRUE, (const
       GLfloat*)TM.GetTSRVC());
                    glUniform4f(PixelColorID,color[0],color[1],color[2],1);
                    glUniform1f(PointSizeID, 5.0);
                    glEnableVertexAttribArray(positionID);
                    glDrawArrays(GL_POINTS,0,1);
                    glDisableVertexAttribArray(positionID);*/

    sphere->SetScale(radius, radius, radius);
    sphere->SetPosition(position[0], position[1], position[2]);
    sphere->Render(cam);
}

void PointLight::SetPos(glm::vec3 pos) {
    position[0] = pos.x;
    position[1] = pos.y;
    position[2] = pos.z;
}

void PointLight::SetCol(glm::vec3 col) {
    color[0] = col.x;
    color[1] = col.y;
    color[2] = col.z;

    radius = CalcSphereSize();
}
float PointLight::CalcSphereSize() {
    return 8.0f *
               sqrtf( glm::length(glm::vec3(color[0], color[1], color[2])) * power) +
           1.0f;
}

Line::Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color) {
    char* vertexShaderSorceCode = ReadFile("shaders/lightVS.vsh");
    char* fragmentShaderSourceCode = ReadFile("shaders/lightFS.fsh");
    shaderProgram = make_shared<Shader>();
    shaderProgram->AddShader(vertexShaderSorceCode, VertexShader);
    shaderProgram->AddShader(fragmentShaderSourceCode, FragmnetShader);
    shaderProgram->Init();
    /*GLuint vertexShaderID=MakeVertexShader(vertexShaderSorceCode);
    GLuint fragmentShaderID=MakeFragmentShader(fragmentShaderSourceCode);
    shaderProgramID=MakeShaderProgram(vertexShaderID, fragmentShaderID);*/

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_STATIC_DRAW);

    pos[0] = pos1.x;
    pos[1] = pos1.y;
    pos[2] = pos1.z;
    pos[3] = pos2.x;
    pos[4] = pos2.y;
    pos[5] = pos2.z;

    col[0] = color.x;
    col[1] = color.y;
    col[2] = color.z;
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);
    positionID = shaderProgram->GetAttribLocation("position");
    gWorldID = shaderProgram->GetUniformLocation("gWVCP");
    PixelColorID = shaderProgram->GetUniformLocation("PixelColor");
    PointSizeID = shaderProgram->GetUniformLocation("size");
}
Line::Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color,
           shared_ptr<Shader> shader) {
    // shaderProgramID=shader;
    shaderProgram = shader;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_STATIC_DRAW);

    pos[0] = pos1.x;
    pos[1] = pos1.y;
    pos[2] = pos1.z;
    pos[3] = pos2.x;
    pos[4] = pos2.y;
    pos[5] = pos2.z;

    col[0] = color.x;
    col[1] = color.y;
    col[2] = color.z;
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);
    positionID = shaderProgram->GetAttribLocation("position");
    gWorldID = shaderProgram->GetUniformLocation("gWVCP");
    PixelColorID = shaderProgram->GetUniformLocation("PixelColor");
    PointSizeID = shaderProgram->GetUniformLocation("size");
}
Line::~Line() {}
// void Line::Render(Camera* pGameCamera, int width, int height)
void Line::Render(Camera* cam) {
    Assistant TM;
    TM.WorldPos(0, 0, 0);
    TM.SetCamera(cam->GetPos(), cam->GetTarget(), cam->GetUp());
    TM.SetPerspectiveProj(cam->GetFov(), cam->GetWidth(), cam->GetHeight(),
                          cam->GetZNear(), cam->GetZFar());

    shaderProgram->Use();
    // glUseProgram(shaderProgramID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glUniformMatrix4fv(gWorldID, 1, GL_TRUE, glm::value_ptr(TM.GetTSRVC()));
    glUniform1f(PointSizeID, 1.5);
    glUniform4f(PixelColorID, col[0], col[1], col[2], 1.0);
    glEnableVertexAttribArray(positionID);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableVertexAttribArray(positionID);
}
shared_ptr<Shader> Line::GetShader() { return shaderProgram; }
