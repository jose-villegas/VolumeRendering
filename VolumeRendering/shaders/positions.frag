#version 330
in vec3 modelPosition;

layout (location=0) out vec4 positions;

void main(void)
{
    positions = vec4(modelPosition, 1.0);
}