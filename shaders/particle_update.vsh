#version 460 core

// Moves each particle one step and, when its lifetime is over, gives it a
// new birth at a random place in the emitter box. Runs with the rasterizer
// off; transform feedback captures the outputs, laid out like the input.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inAge;
layout(location = 2) in vec3 inVelocity;
layout(location = 3) in float inBirths;

layout(xfb_buffer = 0, xfb_stride = 32) out;
layout(xfb_buffer = 0, xfb_offset = 0) out vec3 outPosition;
layout(xfb_buffer = 0, xfb_offset = 12) out float outAge;
layout(xfb_buffer = 0, xfb_offset = 16) out vec3 outVelocity;
layout(xfb_buffer = 0, xfb_offset = 28) out float outBirths;

uniform float deltaSeconds;
uniform float lifetime;
uniform vec3 boxMin;
uniform vec3 boxMax;
uniform vec3 velocity;
uniform vec3 velocityJitter;
uniform vec3 acceleration;
// True on the first update: every particle starts at a random age.
uniform bool seeding;

uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

float Random(uint seed)
{
    return float(Hash(seed) >> 8) / 16777216.0;
}

vec3 Random3(uint seed)
{
    return vec3(Random(seed), Random(seed ^ 0x9e3779b9U),
                Random(seed ^ 0x85ebca6bU));
}

void Bear(uint seed, out vec3 position, out vec3 newVelocity)
{
    position = mix(boxMin, boxMax, Random3(seed));
    newVelocity = velocity + velocityJitter * (Random3(Hash(seed)) * 2.0 - 1.0);
}

void main()
{
    uint id = uint(gl_VertexID);
    if (seeding) {
        uint seed = Hash(id ^ 0x2545f491U);
        Bear(seed, outPosition, outVelocity);
        outAge = lifetime * Random(Hash(seed ^ 0x68e31da4U));
        outBirths = 1.0;
        return;
    }
    vec3 newVelocity = inVelocity + acceleration * deltaSeconds;
    vec3 position = inPosition + newVelocity * deltaSeconds;
    float age = inAge + deltaSeconds;
    float births = inBirths;
    if (age >= lifetime) {
        age = mod(age - lifetime, lifetime);
        births += 1.0;
        Bear(Hash(id ^ Hash(uint(births))), position, newVelocity);
    }
    outPosition = position;
    outAge = age;
    outVelocity = newVelocity;
    outBirths = births;
}
