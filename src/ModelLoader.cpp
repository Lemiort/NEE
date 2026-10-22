#include "ModelLoader.hpp"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <spdlog/spdlog.h>

#include <assimp/Importer.hpp>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <system_error>
#include <utility>

bool LoadModel(const std::string& path, ModelData& model) {
    std::error_code pathError;
    const auto resolvedPath = std::filesystem::absolute(path, pathError);
    const std::string modelPath =
        pathError ? path : resolvedPath.string();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                  aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);
    if (scene == nullptr || scene->mRootNode == nullptr ||
        scene->mNumMeshes == 0) {
        spdlog::error("Unable to import model '{}': {}", modelPath,
                      importer.GetErrorString());
        return false;
    }

    ModelData imported;
    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes;
         ++meshIndex) {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        if (mesh == nullptr || mesh->mNumVertices == 0) {
            spdlog::error("Model '{}' contains an empty mesh", modelPath);
            return false;
        }
        if (imported.vertices.size() >
                std::numeric_limits<std::uint32_t>::max() -
                    mesh->mNumVertices ||
            imported.vertices.size() + mesh->mNumVertices >
                imported.vertices.max_size()) {
            spdlog::error("Model '{}' contains too many vertices", modelPath);
            return false;
        }

        const auto vertexOffset =
            static_cast<std::uint32_t>(imported.vertices.size());
        imported.vertices.reserve(imported.vertices.size() +
                                  mesh->mNumVertices);
        for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices;
             ++vertexIndex) {
            const aiVector3D& position = mesh->mVertices[vertexIndex];
            ModelVertex vertex;
            vertex.position = {position.x, position.y, position.z};
            if (mesh->HasNormals()) {
                const aiVector3D& normal = mesh->mNormals[vertexIndex];
                vertex.normal = {normal.x, normal.y, normal.z};
            }
            if (mesh->HasTextureCoords(0)) {
                const aiVector3D& uv = mesh->mTextureCoords[0][vertexIndex];
                vertex.uv = {uv.x, uv.y};
            }
            if (mesh->HasTangentsAndBitangents()) {
                const aiVector3D& tangent = mesh->mTangents[vertexIndex];
                vertex.tangent = {tangent.x, tangent.y, tangent.z};
            }
            imported.vertices.push_back(vertex);
        }

        for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces;
             ++faceIndex) {
            const aiFace& face = mesh->mFaces[faceIndex];
            if (face.mNumIndices != 3) {
                spdlog::error("Model '{}' contains a non-triangle face",
                              modelPath);
                return false;
            }
            if (imported.indices.size() >
                imported.indices.max_size() - face.mNumIndices) {
                spdlog::error("Model '{}' contains too many indices",
                              modelPath);
                return false;
            }
            for (unsigned int index = 0; index < face.mNumIndices; ++index) {
                if (face.mIndices[index] >= mesh->mNumVertices) {
                    spdlog::error(
                        "Model '{}' contains an invalid face index",
                        modelPath);
                    return false;
                }
                imported.indices.push_back(vertexOffset + face.mIndices[index]);
            }
        }
    }

    if (imported.vertices.empty() || imported.indices.empty()) {
        spdlog::error("Model '{}' contains no renderable geometry",
                      modelPath);
        return false;
    }
    model = std::move(imported);
    return true;
}
