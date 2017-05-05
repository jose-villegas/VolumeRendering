#version 420
struct Light 
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
};

layout(binding=0) uniform sampler2D cubeFront;
layout(binding=1) uniform sampler2D cubeBack;
layout(binding=2) uniform sampler3D volume;
layout(binding=3) uniform sampler3D gradients;
layout(binding=4) uniform sampler1D transferFunction;
layout(binding=5) uniform sampler2D bakedNoise;

uniform mat3 ciModelMatrixInverseTranspose;
uniform Light light;

uniform ivec2 threshold;
uniform vec3 stepSize;
uniform int iterations;
uniform bool diffuseShading;
uniform float stepScale;
uniform float gamma;
uniform float exposure;

in vec4 position;

layout (location=0) out vec4 oColor;

// Spheremap Transform for normal encoding. Used in Cry Engine 3, presented by 
// Martin Mittring in "A bit more Deferred", p. 13
// http://www.crytek.com/sites/default/files/A_bit_more_deferred_-_CryEngine3.ppt
vec3 decode (vec2 enc)
{
    vec3 n;
    n.z = dot(enc, enc) * 2.0 - 1.0;
    n.xy = normalize(enc.xy) * sqrt(1.0 - n.z * n.z);
    return n;
}

void main(void)
{
    vec2 texC = position.xy / position.w;
    texC.x = 0.5 * texC.x + 0.5;
    texC.y = 0.5 * texC.y - 0.5;

    vec3 front = texture(cubeFront, texC).xyz;
    vec3 back = texture(cubeBack, texC).xyz;
    
    vec3 dir = normalize(back - front);
    vec4 pos = vec4(front, 0);

    vec4 dst = vec4(0, 0, 0, 0);
    vec4 src = vec4(0, 0, 0, 0);

    vec4 value = vec4(0);

    vec3 step = dir * stepSize;
    
    // jitter ray starting position to reduce artifacts
    pos.xyz += step * texture(bakedNoise, gl_FragCoord.xy / 256).x;

    for(int i = 0; i < iterations; i++)
    {
        pos.w = 0;
        value.a = texture(volume, pos.xyz).x;
        int isoValue = int(255.0f * value.a);

        if(isoValue >= threshold.x && isoValue <= threshold.y)
        {
            // assigned color from transfer function for this density
            src = texture(transferFunction, value.a);

            // oppacity correction
            src.a = 1 - pow((1 - src.a), stepScale / 0.5);

            if(diffuseShading)
            {
                // gradient value
                value.xyz = decode(texture(gradients, pos.xyz).xy);
                // diffuse shading + fake ambient light
                vec3 normal = normalize(ciModelMatrixInverseTranspose * value.xyz);
                vec3 lightDir = normalize(-light.direction);
                float lambert = max(dot(normal, lightDir), 0.0);
                src.rgb = light.diffuse * lambert * src.rgb + light.ambient * src.rgb;
            }

            // front to back blending
            src.rgb *= src.a;
            dst = (1.0 - dst.a) * src + dst;

            // optimization: break from loop on high enough alpha value
            if(dst.a >= 0.95) 
                break;
        }

        pos.xyz += step;

        // out of bounds
        if (pos.x > 1.0 || pos.y > 1.0 || pos.z > 1.0) 
            break;
    }

    // Exposure tone mapping
    vec3 mapped =  vec3(1.0) - exp(-dst.rgb * exposure);
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    oColor = vec4(mapped, 1.0);
}