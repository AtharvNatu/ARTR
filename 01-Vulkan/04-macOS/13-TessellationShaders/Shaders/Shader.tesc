#version 460 core

layout(vertices = 4) out;

layout(binding = 0) uniform uniformData
{
    mat4 mvpMatrix;
    vec4 numberOfLineSegments;
    vec4 numberOfStrips;
    vec4 lineColor;
} ubo;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    gl_TessLevelOuter[0] = ubo.numberOfLineSegments.x;
}
