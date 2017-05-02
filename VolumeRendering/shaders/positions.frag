in vec3 Color;
out vec4 fragmentColor;

void main(void)
{
    fragmentColor = vec4(Color, 1);
}