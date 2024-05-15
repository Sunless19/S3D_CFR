#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;

uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 trainPos;
uniform float exclusionRadius;

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if(texColor.a < 0.1)
        discard;

    float distance = length(FragPos - trainPos);

    // Calculate fog factor
    float fogFactor = clamp((distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    // If the object is within the exclusion radius, reduce the fog factor
    if (distance < exclusionRadius) {
        fogFactor = 0.0; // No fog within the exclusion zone
    }

    vec3 color = mix(texColor.rgb, fogColor, fogFactor);
    FragColor = vec4(color, texColor.a);
}