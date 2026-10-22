#ifndef BILLBOARD_H
#define BILLBOARD_H
#include <glad/gl.h>

#include <glm/ext/vector_float3.hpp>
#include <memory>

#include "Camera.hpp"
#include "RenderableObject.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
class Billboard : public RenderableObject {
public:
    Billboard();
    //  Billboard(GLuint shader);
    explicit Billboard(std::shared_ptr<Shader> shader);
    ~Billboard() override;
    void Init(const char* TexFilename);
    // Render using a reference instead of a raw pointer for safety.
    // Render using a reference instead of a raw pointer for safety.
    void Render(const Camera& cam) override;
    void SetPos(glm::vec3 Pos);
    // GLuint shaderProgramID;
protected:
private:
    // shared_ptr<Shader> shaderProgram;
 GLuint VBO{};
 glm::vec3 Pos;
 Texture2D colorMap;
 GLint colSamplerID{};
 GLint positionID{};
 GLint camViewID{}, camPosID{};
};

#endif  // BILLBOARD_H
