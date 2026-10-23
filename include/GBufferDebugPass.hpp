#ifndef GBUFFER_DEBUG_PASS_HPP_INCLUDED
#define GBUFFER_DEBUG_PASS_HPP_INCLUDED

class Camera;
class GBuffer;

enum class GBufferDebugView { Position, Diffuse, Normal, TextureCoordinates };

class GBufferDebugPass {
public:
    void Render(GBuffer& gBuffer, const Camera& camera,
                GBufferDebugView view) const;
};

#endif  // GBUFFER_DEBUG_PASS_HPP_INCLUDED
