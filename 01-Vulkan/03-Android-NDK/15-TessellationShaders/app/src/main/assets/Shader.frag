#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform uniformData
{ 
    mat4 mvpMatrix;
    vec4 numberOfLineSegments;
    vec4 numberOfStrips;
    vec4 lineColor; 
} ubo;

layout(location = 0) out vec4 FragColor;

void main(void)
{
    // Code
    FragColor = ubo.lineColor;
}
