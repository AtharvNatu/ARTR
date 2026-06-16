#version 460 core
#extension GL_ARB_separate_shader_objects : enable

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
    vec4 position;

    if (ubo.useGPU == 1)
        position = ssbo_gpu.position[gl_VertexIndex];
    else
        position = ssbo_cpu.position[gl_VertexIndex];

    gl_Position = ubo.mvpMatrix * position;
    gl_PointSize = 1.0;
}
