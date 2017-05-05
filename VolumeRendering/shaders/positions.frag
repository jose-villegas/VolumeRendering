#version 330
in vec3 modelPosition;

layout (location=0) out vec3 positions;

void main(void)
{
    positions = modelPosition;
}