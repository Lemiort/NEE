#include "ShaderFunctions.hpp"

#include <glad/gl.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <print>
#include <vector>

char* ReadFile(const char* filename) {
    FILE* fp = nullptr;
    // fopen_s(&fp, filename, "r");
    fp = fopen(filename, "r");
    if (fp == nullptr) {
        // MessageBox( NULL, "Cannot open shader file!", "ERROR",
        //    MB_OK | MB_ICONEXCLAMATION )
        std::print("Cant open shader file!");
        return nullptr;
    }
    fseek(fp, 0, SEEK_END);
    long const file_length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    std::vector<char> buffer(file_length + 1, 0);
    fread(buffer.data(), 1, file_length, fp);
    buffer[file_length] = '\0';
    char* contents = new char[buffer.size()];
    std::memcpy(contents, buffer.data(), buffer.size());
    fclose(fp);
    return contents;
}

GLuint MakeVertexShader(const char* source) {
    GLuint const vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    if (vertexShaderID == 0) {
        std::println(stderr, "\nError creating vertex shader");
        exit(1);
    }
    glShaderSource(vertexShaderID, 1, static_cast<const GLchar**>(&source),
                   nullptr);
    glCompileShader(vertexShaderID);
    return vertexShaderID;
}

GLuint MakeFragmentShader(const char* source) {
    GLuint const fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragmentShaderID == 0) {
        std::println(stderr, "Error creating fragment shader");
        exit(1);
    }
    glShaderSource(fragmentShaderID, 1, static_cast<const GLchar**>(&source),
                   nullptr);
    glCompileShader(fragmentShaderID);
    return fragmentShaderID;
}

GLuint MakeGeometryShader(const char* source) {
    GLuint const geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
    if (geometryShaderID == 0) {
        std::println(stderr, "Error creating fragment shader");
        exit(1);
    }
    glShaderSource(geometryShaderID, 1, static_cast<const GLchar**>(&source),
                   nullptr);
    glCompileShader(geometryShaderID);
    return geometryShaderID;
}

GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID) {
    FILE* flog = nullptr;
    // fopen_s(&flog,"shaderbuild.log","w");
    flog = fopen("shaderbuild.log", "w");
    GLuint const shaderID = glCreateProgram();
    if (shaderID == 0) {
        std::println(stderr, "Error creating shader program");
        std::println(flog, "Error creating shader program");
        fclose(flog);
        exit(1);
    }
    glAttachShader(shaderID, vertexShaderID);
    glAttachShader(shaderID, fragmentShaderID);
    glLinkProgram(shaderID);
    // ��������

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};
    glGetProgramiv(shaderID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderID, sizeof(ErrorLog), nullptr, ErrorLog);
        std::println(stderr, "Error linking shader program: '{}'", ErrorLog);
        std::println(flog, "Error linking shader program: '{}'", ErrorLog);
        fclose(flog);
        exit(1);
    }

    glValidateProgram(shaderID);
    glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderID, sizeof(ErrorLog), nullptr, ErrorLog);
        std::println(stderr, "Invalid shader program: '{}'", ErrorLog);
        std::println(flog, "Invalid shader program: '{}'", ErrorLog);
        fclose(flog);
        exit(1);
    }
    fclose(flog);
    return shaderID;
}
GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint geometryShaderID,
                         GLuint fragmentShaderID) {
    FILE* flog = nullptr;
    // fopen_s(&flog,"shaderbuild.log","w");
    flog = fopen("shaderbuild.log", "w");
    GLuint const shaderID = glCreateProgram();
    if (shaderID == 0) {
        std::println(stderr, "Error creating shader program");
        std::println(flog, "Error creating shader program");
        fclose(flog);
        exit(1);
    }
    glAttachShader(shaderID, vertexShaderID);
    glAttachShader(shaderID, fragmentShaderID);
    glAttachShader(shaderID, geometryShaderID);
    glLinkProgram(shaderID);
    // ��������

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};
    glGetProgramiv(shaderID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderID, sizeof(ErrorLog), nullptr, ErrorLog);
        std::println(stderr, "Error linking shader program: '{}'", ErrorLog);
        std::println(flog, "Error linking shader program: '{}'", ErrorLog);
        fclose(flog);
        exit(1);
    }

    glValidateProgram(shaderID);
    glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderID, sizeof(ErrorLog), nullptr, ErrorLog);
        std::println(stderr, "Invalid shader program: '{}'", ErrorLog);
        std::println(flog, "Invalid shader program: '{}'", ErrorLog);
        fclose(flog);
        exit(1);
    }
    fclose(flog);
    return shaderID;
}
