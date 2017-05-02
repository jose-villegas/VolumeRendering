#version 400
uniform mat4 ciModelViewProjection;
uniform vec3 scaleFactor;
layout(location = 0) in vec3 position;
out vec3 Color;

void main(void)
{
    gl_Position = ciModelViewProjection * vec4(position * scaleFactor, 1);
    Color = position;
}