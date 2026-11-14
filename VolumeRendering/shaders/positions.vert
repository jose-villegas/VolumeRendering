#version 330
uniform mat4 ciModelViewProjection;

layout(location = 0) in vec3 position;

out vec3 modelPosition;

void main(void)
{
    gl_Position = ciModelViewProjection * vec4(position, 1);
    modelPosition = position;
}