#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

layout(location = 0) out vec3 transformedNormals;
layout(location = 1) out vec3 lightDirection;
layout(location = 2) out vec3 viewerVector;

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

    // Key Press Related Uniform
    uint keyPressed;

} ubo;

void main(void)
{
    // Code
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vPosition;

    if (ubo.keyPressed == 1)
    {
        vec4 eyeCoordinates = ubo.viewMatrix * ubo.modelMatrix * vPosition;
        mat3 normalMatrix = mat3(ubo.viewMatrix * ubo.modelMatrix);
        transformedNormals = normalMatrix * vNormal;
        lightDirection = vec3(ubo.lightPosition - eyeCoordinates);
        viewerVector = -eyeCoordinates.xyz;
    }
}
