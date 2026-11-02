#ifndef VERTEX_ARRAY_HPP_INCLUDED
#define VERTEX_ARRAY_HPP_INCLUDED

#include <glad/gl.h>

#include <stdexcept>
#include <utility>

// Owns the vertex input state for one draw layout. All setup uses DSA and can
// run without changing the VAO or buffer bindings of another renderer.
class VertexArray {
public:
    VertexArray() {
        glCreateVertexArrays(1, &id_);
        if (id_ == 0) throw std::runtime_error("Unable to create vertex array");
    }
    ~VertexArray() {
        if (id_ != 0) glDeleteVertexArrays(1, &id_);
    }
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&& other) noexcept
        : id_(std::exchange(other.id_, 0)) {}
    VertexArray& operator=(VertexArray&& other) noexcept {
        if (this != &other) {
            if (id_ != 0) glDeleteVertexArrays(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    void SetBuffer(GLuint binding, GLuint buffer, GLsizei stride,
                   GLintptr offset = 0) const {
        glVertexArrayVertexBuffer(id_, binding, buffer, offset, stride);
    }
    void SetAttribute(GLint location, GLuint binding, GLint components,
                      GLuint relativeOffset = 0) const {
        // Linkers may remove inputs unused by a particular pass shader.
        if (location < 0) return;
        const auto attribute = static_cast<GLuint>(location);
        glVertexArrayAttribFormat(id_, attribute, components, GL_FLOAT,
                                  GL_FALSE, relativeOffset);
        glVertexArrayAttribBinding(id_, attribute, binding);
        glEnableVertexArrayAttrib(id_, attribute);
    }
    void SetDivisor(GLuint binding, GLuint divisor) const {
        glVertexArrayBindingDivisor(id_, binding, divisor);
    }
    void SetIndices(GLuint buffer) const {
        glVertexArrayElementBuffer(id_, buffer);
    }
    void Bind() const { glBindVertexArray(id_); }

private:
    GLuint id_{};
};

#endif  // VERTEX_ARRAY_HPP_INCLUDED
