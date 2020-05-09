#version 460

out float depth;
layout(location = 0) out vec4 fColor0;
layout(location = 1) out vec4 fColor1;
layout(location = 2) out vec4 fColor2;
layout(location = 3) out vec4 fColor3;
// out vec4 fColor;

void main() {
    depth = gl_FragCoord.z;
    fColor0 = gl_FragCoord;
    fColor1 = vec4(1.0, 0.0, 0.0, 1.0);
    fColor2 = vec4(1.0, 1.0, 0.0, 1.0);
    fColor3 = vec4(0.0, 0.0, 1.0, 1.0);
    // fColor=vec4(vec3(gl_FragCoord.z),1.0);
}