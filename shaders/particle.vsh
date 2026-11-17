#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec3 inVelocity;

out vec3 vPosition;
out vec3 vVelocity;

void main()
{
    vPosition = inPosition;
    vVelocity = inVelocity;
}
