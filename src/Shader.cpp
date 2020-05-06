#include "Shader.h"

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
        case VertexShader:
            if (!source.empty())
                vShaderFileName = source;
            else
                vShaderFileName = "none";
            vShader = MakeVertexShader(source);
            break;
        case FragmnetShader:
            if (!source.empty())
                fShaderFileName = source;
            else
                fShaderFileName = "none";
            fShader = MakeFragmentShader(source);
            break;
        case GeometryShader:
            if (!source.empty())
                gShaderFileName = source;
            else
                gShaderFileName = "none";
            gShader = MakeGeometryShader(source);
            break;
    }
}
void Shader::Init() {
    FILE* flog;
    // fopen_s(&flog,"shaderbuild.log","w");
    flog = fopen("shaderbuild.log", "w");
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        fprintf(flog, "Error creating shader program\n");
        fclose(flog);
        exit(1);
    }
    if (vShader != 0) glAttachShader(shaderProgramID, vShader);
    if (fShader != 0) glAttachShader(shaderProgramID, fShader);
    if (gShader != 0) glAttachShader(shaderProgramID, gShader);
    glLinkProgram(shaderProgramID);
    // проверка

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        fprintf(
            flog,
            "Error linking shader program: at files '%s', '%s', '%s', '%s'\n",
            vShaderFileName.c_str(), gShaderFileName.c_str(),
            fShaderFileName.c_str(), ErrorLog);
        fclose(flog);
        exit(1);
    }

    glValidateProgram(shaderProgramID);
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        // fprintf(flog, "Invalid shader program: '%s'\n", ErrorLog);
        fprintf(flog,
                "Invalid shader program: at files '%s', '%s', '%s', '%s'\n",
                vShaderFileName.c_str(), gShaderFileName.c_str(),
                fShaderFileName.c_str(), ErrorLog);
        fclose(flog);
        exit(1);
    }
    fclose(flog);
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
