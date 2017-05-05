#version 420
uniform mat4 ciModelViewProjection;

in vec4 ciPosition;
in vec2 ciTexCoord0;

out vec2 uvs;

void main()
{
    // fs quad uv coords
    uvs = ciTexCoord0;
    // final drawing pos
    gl_Position = ciModelViewProjection * ciPosition;
}