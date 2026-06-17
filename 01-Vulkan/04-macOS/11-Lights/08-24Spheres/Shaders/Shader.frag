#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 transformedNormals;
layout(location = 1) in vec3 lightDirection;
layout(location = 2) in vec3 viewerVector;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform ubo 
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

    // Key Press Related Uniform
    uint keyPressed;

} uniformData;

layout(push_constant) uniform PushModel
{
    vec4 materialAmbient;
    vec4 materialDiffuse;
    vec4 materialSpecular;
    float materialShininess;

} pushData;

void main(void)
{
    // Code
    vec3 phong_ads_light;

    if (uniformData.keyPressed == 1)
    {
        vec4 ambient = uniformData.lightAmbient * pushData.materialAmbient;
        vec3 normalizedTransformedNormals = normalize(transformedNormals);
        vec3 normalizedLightDirection = normalize(lightDirection);
        vec4 diffuse = uniformData.lightDiffuse * pushData.materialDiffuse * max(dot(normalizedLightDirection, normalizedTransformedNormals), 0.0);
        vec3 reflectionVector = reflect(-normalizedLightDirection, normalizedTransformedNormals);
        vec3 normalizedViewerVector = normalize(viewerVector);
        vec4 specular = uniformData.lightSpecular * pushData.materialSpecular * pow(max(dot(reflectionVector, normalizedViewerVector), 0.0), pushData.materialShininess);

        phong_ads_light = vec3(ambient) + vec3(diffuse) + vec3(specular);
    }
    else
    {
        phong_ads_light = vec3(1.0, 1.0, 1.0);
    }

    FragColor = vec4(phong_ads_light, 1.0);
}
