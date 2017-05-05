#version 330
uniform mat4 ciModelViewProjection;
uniform vec3 scaleFactor;

layout(location = 0) in vec3 position;

out vec3 modelPosition;

void main(void)
{
    gl_Position = ciModelViewProjection * vec4(position * scaleFactor, 1);
    modelPosition = position;
}