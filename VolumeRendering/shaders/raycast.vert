#version 420
uniform mat4 ciModelViewProjection;
uniform vec3 scaleFactor;
layout(location = 0) in vec3 v_position;
out vec4 position;

void main(void)
{
    gl_Position = ciModelViewProjection * vec4(v_position * scaleFactor, 1);
    position = gl_Position;
}