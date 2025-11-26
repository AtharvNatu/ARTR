#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 out_texcoords;
layout(location = 1) in vec3 out_transformedNormals;
layout(location = 2) in vec3 out_lightDirection;
layout(location = 3) in vec3 out_viewerVector;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform uniformData 
{
    // Matrices Related Uniforms
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;

    // Light Related Uniforms
    vec4 lightAmbient;
    vec4 lightDiffuse;
    vec4 lightSpecular;
    vec4 lightPosition;

    // Material Related Uniforms
    vec4 materialAmbient;
    vec4 materialDiffuse;
    vec4 materialSpecular;
    float materialShininess;

} ubo;

layout(binding = 1) uniform sampler2D utextureSampler;

void main(void)
{
    // Code
    vec4 ambient = ubo.lightAmbient * ubo.materialAmbient;
    vec3 normalizedTransformedNormals = normalize(out_transformedNormals);
    vec3 normalizedLightDirection = normalize(out_lightDirection);
    vec4 diffuse = ubo.lightDiffuse * ubo.materialDiffuse * max(dot(normalizedLightDirection, normalizedTransformedNormals), 0.0);
    vec3 reflectionVector = reflect(-normalizedLightDirection, normalizedTransformedNormals);
    vec3 normalizedViewerVector = normalize(out_viewerVector);
    vec4 specular = ubo.lightSpecular * ubo.materialSpecular * pow(max(dot(reflectionVector, normalizedViewerVector), 0.0), ubo.materialShininess);

    vec3 phong_ads_light = vec3(ambient) + vec3(diffuse) + vec3(specular);

    FragColor = vec4(phong_ads_light, 1.0);
    // FragColor = vec4(phong_ads_light * vec3(texture(utextureSampler, out_texcoords)), 1.0);
}
