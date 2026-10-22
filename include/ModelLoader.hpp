#ifndef MODEL_LOADER_H_INCLUDED
#define MODEL_LOADER_H_INCLUDED

#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct ModelVertex {
    std::array<float, 3> position{};
    std::array<float, 3> normal{};
    std::array<float, 2> uv{};
    std::array<float, 3> tangent{};
};

struct ModelData {
    std::vector<ModelVertex> vertices;
    std::vector<std::uint32_t> indices;
};

bool LoadModel(const std::string& path, ModelData& model);

#endif  // MODEL_LOADER_H_INCLUDED
