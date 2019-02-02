#ifndef MESHWIREDMESH_H
#define MESHWIREDMESH_H

#include <glm/glm.hpp>  // vec2, vec3, mat4, radians
#include <vector>

struct WiredMesh {
    std::vector<glm::vec3> vertexes;
    std::vector<glm::ivec2> lines;
};

#endif /* MESHWIREDMESH_H */
