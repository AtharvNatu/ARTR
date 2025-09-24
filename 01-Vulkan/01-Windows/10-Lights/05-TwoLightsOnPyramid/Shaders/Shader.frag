#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 transformedNormals;
layout(location = 1) in vec3 lightDirection[2];
layout(location = 4) in vec3 viewerVector;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform ubo 
{
    // Matrices Related Uniforms
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;

    // Light Related Uniforms
    vec4 lightAmbient[2];
    vec4 lightDiffuse[2];
    vec4 lightSpecular[2];
    vec4 lightPosition[2];

    // Material Related Uniforms
    vec4 materialAmbient;
    vec4 materialDiffuse;
    vec4 materialSpecular;
    float materialShininess;

    // Key Press Related Uniform
    uint keyPressed;

} uniformData;

void main(void)
{
    // Code
    vec3 phong_ads_light = vec3(0.0);

    if (uniformData.keyPressed == 1)
    {
        vec4 ambient[2];
        vec4 diffuse[2];
        vec4 specular[2];
        vec3 normalizedLightDirection[2];
        vec3 reflectionVector[2];

        vec3 normalizedTransformedNormals = normalize(transformedNormals);
        vec3 normalizedViewerVector = normalize(viewerVector);

        for (int i = 0; i < 2; i++)
        {
            ambient[i] = uniformData.lightAmbient[i] * uniformData.materialAmbient;
            normalizedLightDirection[i] = normalize(lightDirection[i]);
            diffuse[i] = uniformData.lightDiffuse[i] * uniformData.materialDiffuse * max(dot(normalizedLightDirection[i], normalizedTransformedNormals), 0.0);
            reflectionVector[i] = reflect(-normalizedLightDirection[i], normalizedTransformedNormals);
            specular[i] = uniformData.lightSpecular[i] * uniformData.materialSpecular * pow(max(dot(reflectionVector[i], normalizedViewerVector), 0.0), uniformData.materialShininess);
            
            phong_ads_light += vec3(ambient[i]) + vec3(diffuse[i]) + vec3(specular[i]);
        }
    }
    else
    {
        phong_ads_light = vec3(1.0, 1.0, 1.0);
    }

    FragColor = vec4(phong_ads_light, 1.0);
}
