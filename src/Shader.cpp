#include "Shader.hpp"

#include <glad/gl.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "spdlog/spdlog.h"

namespace {
constexpr std::array<char, 8> kCacheMagic{'N', 'E', 'E', 'G',
                                          'L', 'B', 'I', 'N'};
constexpr std::uint32_t kCacheVersion = 1;
constexpr std::uint64_t kFnvOffset = 14695981039346656037ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;

void HashBytes(std::uint64_t& hash, std::string_view bytes) {
    for (const unsigned char byte : bytes) {
        hash ^= byte;
        hash *= kFnvPrime;
    }
}

std::string_view GlString(GLenum name) {
    const GLubyte* value = glGetString(name);
    return value == nullptr
               ? std::string_view{}
               : std::string_view{reinterpret_cast<const char*>(value)};
}

std::uint64_t ShaderFingerprint(std::string_view vertex,
                                std::string_view geometry,
                                std::string_view fragment) {
    std::uint64_t hash = kFnvOffset;
    HashBytes(hash, "vertex\0");
    HashBytes(hash, vertex);
    HashBytes(hash, "geometry\0");
    HashBytes(hash, geometry);
    HashBytes(hash, "fragment\0");
    HashBytes(hash, fragment);
    HashBytes(hash, GlString(GL_VENDOR));
    HashBytes(hash, GlString(GL_RENDERER));
    HashBytes(hash, GlString(GL_VERSION));
    return hash;
}

std::filesystem::path CachePath(std::uint64_t fingerprint) {
    std::ostringstream filename;
    filename << std::hex << std::setfill('0') << std::setw(16) << fingerprint
             << ".bin";
    return std::filesystem::path{"shader-cache"} / filename.str();
}

bool SupportsProgramBinaries() {
    if (glGetProgramBinary == nullptr || glProgramBinary == nullptr ||
        glProgramParameteri == nullptr) {
        return false;
    }
    GLint formatCount = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formatCount);
    return formatCount > 0;
}

GLuint CompileShader(std::string_view source, GLenum type) {
    const GLuint shader = glCreateShader(type);
    if (shader == 0) {
        throw std::runtime_error("Unable to create shader");
    }

    const char* sourceData = source.data();
    const auto sourceLength = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &sourceData, &sourceLength);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_TRUE) {
        return shader;
    }

    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<GLchar> errorLog(
        static_cast<std::size_t>(logLength > 0 ? logLength : 1));
    glGetShaderInfoLog(shader, logLength, nullptr, errorLog.data());
    spdlog::error("Error compiling shader: '{}'", errorLog.data());
    glDeleteShader(shader);
    throw std::runtime_error("Error compiling shader");
}

template <typename Value>
bool ReadValue(std::ifstream& input, Value& value) {
    return static_cast<bool>(
        input.read(reinterpret_cast<char*>(&value), sizeof(value)));
}

template <typename Value>
void WriteValue(std::ofstream& output, const Value& value) {
    output.write(reinterpret_cast<const char*>(&value), sizeof(value));
}
}  // namespace

std::string ReadFile(const std::filesystem::path& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        spdlog::error("Failed to open file: {}", filepath.string());
        throw std::runtime_error("Failed to open file: " + filepath.string());
    }

    const auto fileSize = std::filesystem::file_size(filepath);
    std::string content;
    if (fileSize > static_cast<std::uintmax_t>(content.max_size()) ||
        fileSize > static_cast<std::uintmax_t>(
                       std::numeric_limits<std::streamsize>::max())) {
        throw std::length_error("File is too large to read: " +
                                filepath.string());
    }
    content.resize(static_cast<std::size_t>(fileSize));
    file.read(content.data(), static_cast<std::streamsize>(fileSize));
    if (!file) {
        throw std::runtime_error("Failed to read file: " + filepath.string());
    }
    return content;
}

Shader::Shader() = default;

void Shader::AddShader(const std::string& source, ShaderType type) {
    if (initialized || shaderProgramID != 0) {
        throw std::logic_error("Cannot add a shader after program linking");
    }
    switch (type) {
        case VertexShader:
            vertexSource_ = source;
            break;
        case FragmnetShader:
            fragmentSource_ = source;
            break;
        case GeometryShader:
            geometrySource_ = source;
            break;
    }
}

bool Shader::TryLoadCachedProgram() {
    if (!SupportsProgramBinaries()) {
        return false;
    }

    const auto fingerprint =
        ShaderFingerprint(vertexSource_, geometrySource_, fragmentSource_);
    const auto path = CachePath(fingerprint);
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return false;
    }

    std::array<char, kCacheMagic.size()> magic{};
    std::uint32_t version = 0;
    std::uint32_t format = 0;
    std::uint64_t storedFingerprint = 0;
    std::uint64_t binarySize = 0;
    input.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (!ReadValue(input, version) || !ReadValue(input, format) ||
        !ReadValue(input, storedFingerprint) || !ReadValue(input, binarySize) ||
        magic != kCacheMagic || version != kCacheVersion ||
        storedFingerprint != fingerprint || binarySize == 0 ||
        binarySize >
            static_cast<std::uint64_t>(std::numeric_limits<GLsizei>::max())) {
        return false;
    }

    std::vector<std::byte> binary(static_cast<std::size_t>(binarySize));
    if (!input.read(reinterpret_cast<char*>(binary.data()),
                    static_cast<std::streamsize>(binary.size()))) {
        return false;
    }

    shaderProgramID = glCreateProgram();
    glProgramBinary(shaderProgramID, static_cast<GLenum>(format), binary.data(),
                    static_cast<GLsizei>(binary.size()));
    GLint linked = GL_FALSE;
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &linked);
    if (linked == GL_TRUE) {
        initialized = true;
        spdlog::debug("Loaded shader program cache '{}'", path.string());
        return true;
    }

    glDeleteProgram(shaderProgramID);
    shaderProgramID = 0;
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    spdlog::warn("Rejected stale shader program cache '{}'; recompiling",
                 path.string());
    return false;
}

void Shader::CompileAndAttachShaders() {
    if (!vertexSource_.empty()) {
        vShader = CompileShader(vertexSource_, GL_VERTEX_SHADER);
        glAttachShader(shaderProgramID, vShader);
    }
    if (!fragmentSource_.empty()) {
        fShader = CompileShader(fragmentSource_, GL_FRAGMENT_SHADER);
        glAttachShader(shaderProgramID, fShader);
    }
    if (!geometrySource_.empty()) {
        gShader = CompileShader(geometrySource_, GL_GEOMETRY_SHADER);
        glAttachShader(shaderProgramID, gShader);
    }
}

void Shader::SaveProgramBinary() const {
    if (!SupportsProgramBinaries()) {
        return;
    }

    GLint binaryLength = 0;
    glGetProgramiv(shaderProgramID, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
    if (binaryLength <= 0) {
        return;
    }

    std::vector<std::byte> binary(static_cast<std::size_t>(binaryLength));
    GLenum format = 0;
    GLsizei written = 0;
    glGetProgramBinary(shaderProgramID, binaryLength, &written, &format,
                       binary.data());
    if (written <= 0) {
        return;
    }

    const auto fingerprint =
        ShaderFingerprint(vertexSource_, geometrySource_, fragmentSource_);
    const auto path = CachePath(fingerprint);
    std::error_code directoryError;
    std::filesystem::create_directories(path.parent_path(), directoryError);
    if (directoryError) {
        spdlog::warn("Cannot create shader cache directory: {}",
                     directoryError.message());
        return;
    }

    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        spdlog::warn("Cannot write shader program cache '{}'", path.string());
        return;
    }
    output.write(kCacheMagic.data(),
                 static_cast<std::streamsize>(kCacheMagic.size()));
    WriteValue(output, kCacheVersion);
    WriteValue(output, static_cast<std::uint32_t>(format));
    WriteValue(output, fingerprint);
    WriteValue(output, static_cast<std::uint64_t>(written));
    output.write(reinterpret_cast<const char*>(binary.data()), written);
}

void Shader::Link() {
    if (initialized || TryLoadCachedProgram()) {
        return;
    }

    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        throw std::runtime_error("Error creating shader program");
    }
    if (SupportsProgramBinaries()) {
        glProgramParameteri(shaderProgramID, GL_PROGRAM_BINARY_RETRIEVABLE_HINT,
                            GL_TRUE);
    }
    CompileAndAttachShaders();
    glLinkProgram(shaderProgramID);

    GLint success = GL_FALSE;
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(shaderProgramID, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> errorLog(
            static_cast<std::size_t>(logLength > 0 ? logLength : 1));
        glGetProgramInfoLog(shaderProgramID, logLength, nullptr,
                            errorLog.data());
        spdlog::error("Error linking shader program: '{}'", errorLog.data());
        throw std::runtime_error("Error linking shader program");
    }

    glValidateProgram(shaderProgramID);
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &success);
    if (success != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(shaderProgramID, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> errorLog(
            static_cast<std::size_t>(logLength > 0 ? logLength : 1));
        glGetProgramInfoLog(shaderProgramID, logLength, nullptr,
                            errorLog.data());
        spdlog::error("Invalid shader program: '{}'", errorLog.data());
        throw std::runtime_error("Invalid shader program");
    }

    initialized = true;
    SaveProgramBinary();
}

void Shader::Use() {
    if (!initialized) {
        Link();
    }
    glUseProgram(shaderProgramID);
}

GLint Shader::GetAttribLocation(const std::string& attribute) const {
    return glGetAttribLocation(shaderProgramID, attribute.c_str());
}

GLint Shader::GetUniformLocation(const std::string& attribute) const {
    return glGetUniformLocation(shaderProgramID, attribute.c_str());
}

Shader::~Shader() {
    if (vShader != 0) glDeleteShader(vShader);
    if (fShader != 0) glDeleteShader(fShader);
    if (gShader != 0) glDeleteShader(gShader);
    if (shaderProgramID != 0) glDeleteProgram(shaderProgramID);
}
