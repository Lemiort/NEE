#version 330

uniform vec3 s_vCamPos;

out vec3 fE;
out vec4 screenSpacePos;

void main()
{
    const vec2 positions[3] = vec2[3](
        vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    screenSpacePos = gl_Position;
    fE = s_vCamPos;
}
