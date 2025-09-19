#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

layout(location = 0) out vec3 phong_ads_light;

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
    gl_Position = uniformData.projectionMatrix * uniformData.viewMatrix * uniformData.modelMatrix * vPosition;

    if (uniformData.keyPressed == 1)
    {
        vec4 ambient[2];
        vec4 diffuse[2];
        vec4 specular[2];
        vec3 lightDirection[2];
        vec3 reflectionVector[2];

        vec4 eyeCoordinates = uniformData.viewMatrix * uniformData.modelMatrix * vPosition;
        mat3 normalMatrix = mat3(uniformData.viewMatrix * uniformData.modelMatrix);
        vec3 transformedNormals = normalize(normalMatrix * vNormal);
        vec3 viewerVector = normalize(-eyeCoordinates.xyz);

        for (int i = 0; i < 2; i++)
        {
            ambient[i] = uniformData.lightAmbient[i] * uniformData.materialAmbient;
            lightDirection[i] = normalize(vec3(uniformData.lightPosition[i] - eyeCoordinates));
            diffuse[i] = uniformData.lightDiffuse[i] * uniformData.materialDiffuse * max(dot(lightDirection[i], transformedNormals), 0.0);
            reflectionVector[i] = reflect(-lightDirection[i], transformedNormals);
            specular[i] = uniformData.lightSpecular[i] * uniformData.materialSpecular * pow(max(dot(reflectionVector[i], viewerVector), 0.0), uniformData.materialShininess);

            phong_ads_light += vec3(ambient[i]) + vec3(diffuse[i]) + vec3(specular[i]);
        }
    }
    else
    {
        phong_ads_light = vec3(1.0, 1.0, 1.0);
    }

}