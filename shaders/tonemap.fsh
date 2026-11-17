#version 460 core

// Maps the HDR light of the frame to the screen: add the bloom, scale by
// exposure, then compress with the tone-mapping operator. 0 clamps, 1 is
// Reinhard, 2 is the ACES filmic fit by Krzysztof Narkowicz.
uniform sampler2D hdrSampler;
uniform sampler2D bloomSampler;
uniform float bloomStrength;
uniform float exposure;
uniform int toneMapOperator;

out vec4 fColor;

vec3 Aces(vec3 x)
{
    return (x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14);
}

void main()
{
    vec3 color = texelFetch(hdrSampler, ivec2(gl_FragCoord.xy), 0).rgb;
    if (bloomStrength > 0.0) {
        vec2 uv = gl_FragCoord.xy / vec2(textureSize(hdrSampler, 0));
        color += texture(bloomSampler, uv).rgb * bloomStrength;
    }
    color *= exposure;
    if (toneMapOperator == 1)
        color = color / (1.0 + color);
    else if (toneMapOperator == 2)
        color = Aces(color);
    fColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
