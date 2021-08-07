#include "Shader.h"

#include <spdlog/spdlog.h>

#include "ShaderFunctions.h"

Shader::Shader() {
    // ctor
    vShader = 0;
    fShader = 0;
    gShader = 0;
    shaderProgramID = 0;
    initialized = false;
}
void Shader::AddShader(const std::string& source, ShaderType type) {
    switch (type) {
        case ShaderType::kVertexShader:
            if (!source.empty())
                vShaderFileName = source;
            else
                vShaderFileName = "none";
            vShader = MakeVertexShader(source);
            break;
        case ShaderType::kFragmentShader:
            if (!source.empty())
                fShaderFileName = source;
            else
                fShaderFileName = "none";
            fShader = MakeFragmentShader(source);
            break;
        case ShaderType::kGeometryShader:
            if (!source.empty())
                gShaderFileName = source;
            else
                gShaderFileName = "none";
            gShader = MakeGeometryShader(source);
            break;
    }
}
void Shader::Init() {
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        spdlog::error("Error creating  shader program");
        exit(1);
    }
    if (vShader != 0) glAttachShader(shaderProgramID, vShader);
    if (fShader != 0) glAttachShader(shaderProgramID, fShader);
    if (gShader != 0) glAttachShader(shaderProgramID, gShader);
    glLinkProgram(shaderProgramID);

    // check
    GLint Success = 0;
    GLchar errorLog[1024]{0};
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID, sizeof(errorLog), NULL, errorLog);
        spdlog::error("Error linking shader program: at files {}, {}, {}: {}",
                      vShaderFileName, gShaderFileName, fShaderFileName,
                      errorLog);
        exit(1);
    }

    glValidateProgram(shaderProgramID);
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(errorLog), NULL, errorLog);
        spdlog::error("Invalid shader program from files {}, {}, {}: {}",
                      vShaderFileName, gShaderFileName, fShaderFileName,
                      errorLog);
        exit(1);
    }
    // return shaderID;
    initialized = true;
}

void Shader::Use() {
    if (!initialized) Init();
    glUseProgram(shaderProgramID);
}

GLuint Shader::GetAttribLocation(const char* attribute) {
    return glGetAttribLocation(shaderProgramID, attribute);
}

GLuint Shader::GetUniformLocation(const char* attribute) {
    return glGetUniformLocation(shaderProgramID, attribute);
}

Shader::~Shader() {
    // dtor
    if (vShader != 0) glDeleteShader(vShader);
    if (fShader != 0) glDeleteShader(fShader);
    if (gShader != 0) glDeleteShader(gShader);
    if (shaderProgramID != 0) glDeleteProgram(shaderProgramID);
}
