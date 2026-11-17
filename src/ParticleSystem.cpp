#include "ParticleSystem.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Camera.hpp"
#include "Shader.hpp"

static_assert(sizeof(ParticleSystem::Particle) == 32,
              "Particles must match the 32-byte transform feedback layout");

ParticleSystem::ParticleSystem(
    gsl_lite::not_null<std::shared_ptr<Shader>> updateShader,
    gsl_lite::not_null<std::shared_ptr<Shader>> renderShader, std::size_t count)
    : updateShader_(std::move(updateShader)),
      renderShader_(std::move(renderShader)),
      count_(count) {
    if (count_ == 0 ||
        count_ > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()) /
                     sizeof(Particle)) {
        throw std::invalid_argument("Particle count out of range");
    }
    // The first update seeds every particle, so the contents start unused.
    const std::vector<Particle> particles(count_, Particle{});
    const auto bytes = static_cast<GLsizeiptr>(count_ * sizeof(Particle));
    glCreateBuffers(static_cast<GLsizei>(buffers_.size()), buffers_.data());
    glCreateTransformFeedbacks(1, &feedback_);
    for (std::size_t index = 0; index < buffers_.size(); ++index) {
        glNamedBufferStorage(buffers_[index], bytes, particles.data(), 0);
        const VertexArray& vertexArray = vertexArrays_[index];
        vertexArray.SetBuffer(0, buffers_[index], sizeof(Particle));
        vertexArray.SetAttribute(0, 0, 3, offsetof(Particle, position));
        vertexArray.SetAttribute(1, 0, 1, offsetof(Particle, age));
        vertexArray.SetAttribute(2, 0, 3, offsetof(Particle, velocity));
        vertexArray.SetAttribute(3, 0, 1, offsetof(Particle, births));
    }
}

ParticleSystem::~ParticleSystem() {
    glDeleteTransformFeedbacks(1, &feedback_);
    glDeleteBuffers(static_cast<GLsizei>(buffers_.size()), buffers_.data());
}

void ParticleSystem::Update(float deltaSeconds) {
    const std::size_t next = 1 - current_;
    Shader& shader = *updateShader_;
    shader.Use();
    const auto vec3 = [&shader](const char* name, const glm::vec3& value) {
        glUniform3fv(shader.GetUniformLocation(name), 1, glm::value_ptr(value));
    };
    glUniform1f(shader.GetUniformLocation("deltaSeconds"), deltaSeconds);
    glUniform1f(shader.GetUniformLocation("lifetime"), emitter_.lifetime);
    vec3("boxMin", emitter_.boxMin);
    vec3("boxMax", emitter_.boxMax);
    vec3("velocity", emitter_.velocity);
    vec3("velocityJitter", emitter_.velocityJitter);
    vec3("acceleration", emitter_.acceleration);
    glUniform1i(shader.GetUniformLocation("seeding"),
                seeded_ ? GL_FALSE : GL_TRUE);

    glEnable(GL_RASTERIZER_DISCARD);
    vertexArrays_[current_].Bind();
    glTransformFeedbackBufferBase(feedback_, 0, buffers_[next]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback_);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(count_));
    glEndTransformFeedback();
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glDisable(GL_RASTERIZER_DISCARD);

    current_ = next;
    seeded_ = true;
}

void ParticleSystem::Render(const Camera& camera) const {
    if (!seeded_) {
        return;
    }
    Shader& shader = *renderShader_;
    shader.Use();
    const glm::mat4 viewProjection = camera.GetViewProjectionMatrix();
    glUniformMatrix4fv(shader.GetUniformLocation("viewProjection"), 1, GL_FALSE,
                       glm::value_ptr(viewProjection));
    const glm::vec3 position = camera.GetPos();
    glUniform3fv(shader.GetUniformLocation("cameraPosition"), 1,
                 glm::value_ptr(position));
    glUniform1f(shader.GetUniformLocation("width"), look_.width);
    glUniform1f(shader.GetUniformLocation("stretch"), look_.stretch);
    glUniform3fv(shader.GetUniformLocation("color"), 1,
                 glm::value_ptr(look_.color));
    glUniform1f(shader.GetUniformLocation("opacity"), look_.opacity);

    // Quads face the camera either way round, so culling stays off for them.
    const GLboolean culling = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, look_.additive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
    vertexArrays_[current_].Bind();
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(count_));
    if (culling == GL_TRUE) {
        glEnable(GL_CULL_FACE);
    }
}

std::vector<ParticleSystem::Particle> ParticleSystem::ReadParticles() const {
    std::vector<Particle> particles(count_);
    glGetNamedBufferSubData(buffers_[current_], 0,
                            static_cast<GLsizeiptr>(count_ * sizeof(Particle)),
                            particles.data());
    return particles;
}
