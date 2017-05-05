#version 420
layout(binding=0) uniform sampler2D hdrBuffer;

uniform float gamma;
uniform float exposure;

in vec2 uvs;
out vec4 fragmentColor;

vec3 ACESFilm( vec3 x )
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main()
{
    vec3 hdrColor = texture(hdrBuffer, uvs).rgb;
  
    // Exposure tone mapping
    vec3 mapped = ACESFilm(hdrColor * exposure);
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    fragmentColor = vec4(mapped, 1.0);
}