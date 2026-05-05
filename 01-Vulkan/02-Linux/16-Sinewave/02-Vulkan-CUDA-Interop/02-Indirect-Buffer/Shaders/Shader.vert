#version 460 core
#extension GL_ARB_separate_shader_objects : enable

// layout(location = 0) in vec4 vPosition_cpu;
// layout(location = 1) in vec4 vPosition_gpu;

layout(binding = 0) uniform mvpData 
{ 
    mat4 mvpMatrix;
    vec4 color;
    int useGPU;
} ubo;

//* SSBOs
layout(binding = 1) readonly buffer cpuBuffer
{
    vec4 position[];
} ssbo_cpu;

layout(binding = 2) readonly buffer gpuBuffer
{
    vec4 position[];
} ssbo_gpu;

void main(void)
{
    // Code
    vec4 pos;

    if (ubo.useGPU == 1)
        pos = ssbo_gpu.position[gl_VertexIndex];
    else
        pos = ssbo_cpu.position[gl_VertexIndex];

    gl_Position = ubo.mvpMatrix * pos;
    gl_PointSize = 1.0;
}
