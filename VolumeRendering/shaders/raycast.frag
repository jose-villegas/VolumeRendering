#version 420
// Front to back ray marching through the volume. Built twice: with RENDER_TARGETS
// it writes HDR color and normals for the post-process chain, without it the color
// is tone mapped and written straight to the screen.
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
layout(binding=6) uniform sampler3D lightVolume;
layout(binding=7) uniform sampler3D ambientVolume;

uniform mat4 ciModelMatrix;
uniform mat3 ciModelMatrixInverseTranspose;
uniform mat3 ciNormalMatrix;
uniform Light light;

uniform vec2 threshold;
uniform vec3 stepSize;
uniform int iterations;
uniform float stepScale;

// classic lighting: lambert + constant ambient
uniform bool diffuseShading;

// volumetric lighting
uniform bool volumetricLighting;
uniform vec3 eyePosition;
// direction toward the light in texture space, one light volume voxel long
uniform vec3 lightOffset;
uniform float gradientShading;
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform bool volumetricShadows;
uniform bool volumeOcclusion;
uniform bool indirectLight;
uniform float indirectStrength;

#ifndef RENDER_TARGETS
uniform float gamma;
uniform float exposure;
#endif

in vec4 position;

layout (location=0) out vec4 oColor;
#ifdef RENDER_TARGETS
layout (location=1) out vec4 oNormal;
#endif

// direct light, ambient occlusion and indirect light for one sample, world space
vec3 volumetricShading(vec3 albedo, vec3 pos, vec4 gradient)
{
    vec3 toLight = normalize(-light.direction);
    vec3 worldPos = (ciModelMatrix * vec4(pos, 1.0)).xyz;
    vec3 toEye = normalize(eyePosition - worldPos);

    // surface normal points against the gradient, toward lower density. Shading
    // fades from a surface (lambert + specular) to a scattering medium as the
    // gradient magnitude drops, where the normal is noise
    float magnitude = gradient.w;
    float surface = gradientShading * smoothstep(0.005, 0.06, magnitude);
    vec3 normal = normalize(ciModelMatrixInverseTranspose * -gradient.xyz / max(magnitude, 1e-5));

    float lambert = max(dot(normal, toLight), 0.0);
    float diffuse = mix(1.0, lambert, surface);
    vec3 halfway = normalize(toLight + toEye);
    float specular = surface * specularStrength * (shininess + 8.0) / 25.13274 *
                     pow(max(dot(normal, halfway), 0.0), shininess) * step(0.0, lambert);

    float shadow = volumetricShadows ? texture(lightVolume, pos + lightOffset).r : 1.0;
    vec4 ambient = volumeOcclusion || indirectLight ? texture(ambientVolume, pos) : vec4(0.0, 0.0, 0.0, 1.0);
    // a point on an open flat surface sees half of the directions blocked, that counts as unoccluded
    float visibility = volumeOcclusion ? min(2.0 * ambient.a, 1.0) : 1.0;
    vec3 bounce = indirectLight ? ambient.rgb * indirectStrength : vec3(0.0);

    return albedo * (light.diffuse * (diffuse * shadow + bounce) + light.ambient * ambientStrength * visibility) +
           light.diffuse * specular * shadow;
}

void main(void)
{
    vec2 texC = position.xy / position.w;
    texC.x = 0.5 * texC.x + 0.5;
    texC.y = 0.5 * texC.y - 0.5;

    vec3 front = texture(cubeFront, texC).xyz;
    vec3 back = texture(cubeBack, texC).xyz;

    vec3 dir = normalize(back - front);
    float rayLength = length(back - front);
    vec3 pos = front;

    vec4 dst = vec4(0);
    vec4 src = vec4(0);
    vec3 normal = vec3(0);

    vec3 step = dir * stepSize;
    float stepLength = length(step);

    // jitter ray starting position to reduce artifacts
    float travelled = stepLength * texture(bakedNoise, gl_FragCoord.xy / 256).x;
    pos += dir * travelled;

    for(int i = 0; i < iterations && travelled <= rayLength; i++)
    {
        float density = texture(volume, pos).x;

        if(density >= threshold.x && density <= threshold.y)
        {
            // assigned color from transfer function for this density
            src = texture(transferFunction, density);

            // opacity correction
            src.a = 1 - pow((1 - src.a), stepScale / 0.5);

            // outward normal, against the gradient toward lower density
            vec4 gradient = texture(gradients, pos);
            normal = -gradient.xyz / max(length(gradient.xyz), 1e-5);

            if(volumetricLighting)
            {
                src.rgb = volumetricShading(src.rgb, pos, gradient);
            }
            else if(diffuseShading)
            {
                // diffuse shading + fake ambient light
                vec3 wsNormal = normalize(ciModelMatrixInverseTranspose * normal);
                float lambert = max(dot(wsNormal, normalize(-light.direction)), 0.0);
                src.rgb = light.diffuse * lambert * src.rgb + light.ambient * src.rgb;
            }

            // front to back blending
            src.rgb *= src.a;
            dst = (1.0 - dst.a) * src + dst;

            // optimization: break from loop on high enough alpha value
            if(dst.a >= 0.95)
                break;
        }

        pos += step;
        travelled += stepLength;
    }

#ifdef RENDER_TARGETS
    oColor = vec4(dst.rgb, 1.0);
    // view space normal of the last sample
    oNormal = vec4(ciNormalMatrix * normal, 1.0);
#else
    // exposure tone mapping and gamma correction
    vec3 mapped = vec3(1.0) - exp(-dst.rgb * exposure);
    oColor = vec4(pow(mapped, vec3(1.0 / gamma)), 1.0);
#endif
}
