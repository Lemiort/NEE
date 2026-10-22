#include "GBufferDebugPass.hpp"

#include <glad/gl.h>

#include "Camera.hpp"
#include "GBuffer.hpp"

void GBufferDebugPass::Render(GBuffer& gBuffer, const Camera& camera,
                              GBufferDebugView view) const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gBuffer.BindForReading();
    const auto texture = [view] {
        switch (view) {
            case GBufferDebugView::Position:
                return GBuffer::GBUFFER_TEXTURE_TYPE_POSITION;
            case GBufferDebugView::Diffuse:
                return GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE;
            case GBufferDebugView::Normal:
                return GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL;
            case GBufferDebugView::TextureCoordinates:
                return GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD;
        }
        return GBuffer::GBUFFER_TEXTURE_TYPE_POSITION;
    }();
    GBuffer::SetReadBuffer(texture);
    glBlitFramebuffer(0, 0, camera.GetWidth(), camera.GetHeight(), 0, 0,
                      camera.GetWidth(), camera.GetHeight(),
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
