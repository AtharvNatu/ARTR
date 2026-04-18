#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

layout(location = 0) out vec3 out_diffused_light_color;

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
        vec4 ambient = uniformData.lightAmbient * uniformData.materialAmbient;
        vec4 eyeCoordinates = uniformData.viewMatrix * uniformData.modelMatrix * vPosition;
        mat3 normalMatrix = mat3(uniformData.viewMatrix * uniformData.modelMatrix);
        vec3 transformedNormals = normalize(normalMatrix * vNormal);
        vec3 lightDirection = normalize(vec3(uniformData.lightPosition - eyeCoordinates));
        vec4 diffuse = uniformData.lightDiffuse * uniformData.materialDiffuse * max(dot(lightDirection, transformedNormals), 0.0);
        vec3 reflectionVector = reflect(-lightDirection, transformedNormals);
        vec3 viewerVector = normalize(-eyeCoordinates.xyz);
        vec4 specular = uniformData.lightSpecular * uniformData.materialSpecular * pow(max(dot(reflectionVector, viewerVector), 0.0), uniformData.materialShininess);

        out_diffused_light_color = vec3(ambient) + vec3(diffuse) + vec3(specular);
    }
    else
    {
        out_diffused_light_color = vec3(1.0, 1.0, 1.0);
    }

}