#include "Shader.hpp"

#include <glad/gl.h>

#include <cstdio>
#include <cstdlib>
#include <print>

#include "ShaderFunctions.hpp"
Shader::Shader()

{
  // ctor
}
void Shader::AddShader(const char* source, ShaderType type) {
    switch (type) {
        case VertexShader:
            if (source != nullptr) {
                vShaderFileName = source;
            } else {
                vShaderFileName = "none";
            }
            vShader = MakeVertexShader(source);
            break;
        case FragmnetShader:
            if (source != nullptr) {
                fShaderFileName = source;
            } else {
                fShaderFileName = "none";
            }
            fShader = MakeFragmentShader(source);
            break;
        case GeometryShader:
            if (source != nullptr) {
                gShaderFileName = source;
            } else {
                gShaderFileName = "none";
            }
            gShader = MakeGeometryShader(source);
            break;
    }
}
void Shader::Init() {
    FILE* flog = nullptr;
    // fopen_s(&flog,"shaderbuild.log","w");
    flog = fopen("shaderbuild.log", "w");
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        std::println(stderr, "Error creating shader program");
        std::println(flog, "Error creating shader program");
        fclose(flog);
        exit(1);
    }
    if (vShader != 0) {
        glAttachShader(shaderProgramID, vShader);
    }
    if (fShader != 0) {
        glAttachShader(shaderProgramID, fShader);
    }
    if (gShader != 0) {
        glAttachShader(shaderProgramID, gShader);
    }
    glLinkProgram(shaderProgramID);
    // check

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), nullptr,
                            ErrorLog);
        std::println(stderr, "Error linking shader program: '{}'", ErrorLog);
        std::println(
            flog,
            "Error linking shader program: at files '{}', '{}', '{}', '{}'",
            vShaderFileName, gShaderFileName, fShaderFileName, ErrorLog);
        fclose(flog);
        exit(1);
    }

    glValidateProgram(shaderProgramID);
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), nullptr,
                            ErrorLog);
        std::println(stderr, "Invalid shader program: '{}'", ErrorLog);
        // fprintf(flog, "Invalid shader program: '%s'\n", ErrorLog);
        std::println(
            flog, "Invalid shader program: at files '{}', '{}', '{}', '{}'",
            vShaderFileName, gShaderFileName, fShaderFileName, ErrorLog);
        fclose(flog);
        exit(1);
    }
    fclose(flog);
    // return shaderID;
    initialized = true;
}

void Shader::Use() {
    if (!initialized) {
        Init();
    }
    glUseProgram(shaderProgramID);
}

GLuint Shader::GetAttribLocation(const char* attribute) const {
    return glGetAttribLocation(shaderProgramID, attribute);
}

GLuint Shader::GetUniformLocation(const char* attribute) const {
    return glGetUniformLocation(shaderProgramID, attribute);
}

Shader::~Shader() {
    // dtor
    if (vShader != 0) {
        glDeleteShader(vShader);
    }
    if (fShader != 0) {
        glDeleteShader(fShader);
    }
    if (gShader != 0) {
        glDeleteShader(gShader);
    }
    if (shaderProgramID != 0) {
        glDeleteProgram(shaderProgramID);
    }
}
