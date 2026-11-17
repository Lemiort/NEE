#version 460 core

// Halves the source with the 13-tap filter of Jimenez, "Next Generation Post
// Processing in Call of Duty: Advanced Warfare" (2014). The first pass also
// keeps only the light above the threshold, fading in over the soft knee.
uniform sampler2D source;
uniform vec2 sourceTexel;  // 1 / source size
uniform vec2 targetSize;
uniform bool firstPass;
uniform float threshold;
uniform float knee;

out vec4 fColor;

vec3 Bright(vec3 color)
{
    float brightness = max(color.r, max(color.g, color.b));
    float soft = clamp(brightness - threshold + knee, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 1e-5);
    float contribution = max(soft, brightness - threshold) / max(brightness, 1e-5);
    return color * contribution;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / targetSize;
    vec2 d = sourceTexel;
    vec3 a = texture(source, uv + d * vec2(-2.0, 2.0)).rgb;
    vec3 b = texture(source, uv + d * vec2(0.0, 2.0)).rgb;
    vec3 c = texture(source, uv + d * vec2(2.0, 2.0)).rgb;
    vec3 e = texture(source, uv + d * vec2(-2.0, 0.0)).rgb;
    vec3 f = texture(source, uv).rgb;
    vec3 g = texture(source, uv + d * vec2(2.0, 0.0)).rgb;
    vec3 h = texture(source, uv + d * vec2(-2.0, -2.0)).rgb;
    vec3 i = texture(source, uv + d * vec2(0.0, -2.0)).rgb;
    vec3 j = texture(source, uv + d * vec2(2.0, -2.0)).rgb;
    vec3 k = texture(source, uv + d * vec2(-1.0, 1.0)).rgb;
    vec3 l = texture(source, uv + d * vec2(1.0, 1.0)).rgb;
    vec3 m = texture(source, uv + d * vec2(-1.0, -1.0)).rgb;
    vec3 n = texture(source, uv + d * vec2(1.0, -1.0)).rgb;
    vec3 color = f * 0.125 + (a + c + h + j) * 0.03125 +
                 (b + e + g + i) * 0.0625 + (k + l + m + n) * 0.125;
    if (firstPass)
        color = Bright(color);
    fColor = vec4(color, 1.0);
}
