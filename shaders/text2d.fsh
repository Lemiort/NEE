#version 460 core
in vec2 UV1;
uniform vec4 textColor;
uniform sampler2D texSampler;

out vec4 FragColor;

// The font atlas stores a signed distance field in alpha: 0.5 is the glyph
// edge, larger values are inside.
const float kEdge = 0.5;

void main()
{
    float distance = texture(texSampler, UV1).a;
    // Fade over about one screen pixel whatever the text size: fwidth gives
    // how much the distance changes between neighbouring pixels.
    float halfWidth = max(fwidth(distance) * 0.75, 1e-4);
    float coverage = smoothstep(kEdge - halfWidth, kEdge + halfWidth, distance);
    if (coverage <= 0.0) {
        discard;
    }
    FragColor = vec4(textColor.rgb, textColor.a * coverage);
}
