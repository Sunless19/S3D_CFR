#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    float visibility; // Added this line
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

const float density = 0.007;
const float gradient = 1.5;

void main()
{
    vec4 fragPosViewSpace = view * model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    gl_Position = projection * fragPosViewSpace;

    float distance = length(fragPosViewSpace.xyz);
    vs_out.visibility = exp(-pow((distance * density), gradient));
    vs_out.visibility = clamp(vs_out.visibility, 0.0, 1.0);
}