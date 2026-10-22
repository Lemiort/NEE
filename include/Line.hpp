#ifndef LINE_H_INCLUDED
#define LINE_H_INCLUDED
#include <glad/gl.h>

#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <memory>

#include "Camera.hpp"
#include "Shader.hpp"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

class Line {
public:
    /*glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 c1;*/
    //	GLuint shaderProgramID;
    GLint PixelColorID, PointSizeID;
    GLint gWorldID;
    GLint positionID;
    GLuint VBO;
    float col[3];
    float pos[6];
    Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color);
    Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color, GLuint shader);
    ~Line();
    void Render(const Camera& pGameCamera);

private:
    std::shared_ptr<Shader> shaderProgram;
};
#endif  // LINE_H_INCLUDED
