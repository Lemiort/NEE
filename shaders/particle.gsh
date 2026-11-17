#version 460 core

// Turns each particle into a quad facing the camera: a square `width` wide,
// or, with `stretch`, a streak from the particle back along its velocity
// for `stretch` seconds of travel, like a falling raindrop.
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vPosition[];
in vec3 vVelocity[];

uniform mat4 viewProjection;
uniform vec3 cameraPosition;
uniform float width;
uniform float stretch;

void Emit(vec3 position)
{
    gl_Position = viewProjection * vec4(position, 1.0);
    EmitVertex();
}

void main()
{
    vec3 head = vPosition[0];
    vec3 toCamera = normalize(cameraPosition - head);
    vec3 trail = vVelocity[0] * stretch;
    vec3 side = cross(trail, toCamera);
    if (length(side) > 1e-6) {
        side = normalize(side) * (width * 0.5);
        vec3 tail = head - trail;
        Emit(head - side);
        Emit(head + side);
        Emit(tail - side);
        Emit(tail + side);
    } else {
        vec3 up = abs(toCamera.y) > 0.99 ? vec3(1.0, 0.0, 0.0)
                                         : vec3(0.0, 1.0, 0.0);
        vec3 right = normalize(cross(up, toCamera)) * (width * 0.5);
        vec3 top = normalize(cross(toCamera, right)) * (width * 0.5);
        Emit(head - right - top);
        Emit(head + right - top);
        Emit(head - right + top);
        Emit(head + right + top);
    }
    EndPrimitive();
}
