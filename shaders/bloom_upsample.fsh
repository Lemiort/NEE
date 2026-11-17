#version 460 core

// Doubles the source with a 3x3 tent filter; the pass adds it onto the
// larger level, which already holds its own downsampled light.
uniform sampler2D source;
uniform vec2 sourceTexel;  // 1 / source size
uniform vec2 targetSize;
uniform float radius;      // in source texels

out vec4 fColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / targetSize;
    vec2 d = sourceTexel * radius;
    vec3 color = texture(source, uv).rgb * 4.0;
    color += (texture(source, uv + vec2(-d.x, 0.0)).rgb +
              texture(source, uv + vec2(d.x, 0.0)).rgb +
              texture(source, uv + vec2(0.0, -d.y)).rgb +
              texture(source, uv + vec2(0.0, d.y)).rgb) * 2.0;
    color += texture(source, uv + vec2(-d.x, -d.y)).rgb +
             texture(source, uv + vec2(d.x, -d.y)).rgb +
             texture(source, uv + vec2(-d.x, d.y)).rgb +
             texture(source, uv + vec2(d.x, d.y)).rgb;
    fColor = vec4(color / 16.0, 1.0);
}
