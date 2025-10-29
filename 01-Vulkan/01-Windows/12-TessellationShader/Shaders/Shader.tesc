#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(vertices = 4) out;

layout(binding = 0) uniform uniformData
{ 
    mat4 mvpMatrix;
    vec4 numberOfLineSegments;
    vec4 numberOfStrips;
    vec4 lineColor; 
} ubo;

void main(void)
{
    // Code
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    gl_TessLevelOuter[0] = ubo.numberOfStrips.x;
    gl_TessLevelOuter[1] = ubo.numberOfLineSegments.x;
}
