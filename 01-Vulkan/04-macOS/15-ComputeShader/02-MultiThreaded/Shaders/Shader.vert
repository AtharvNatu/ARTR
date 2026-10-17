#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in float vHeight;

layout(binding = 0) uniform uniformData 
{ 
    mat4 mvpMatrix;
    vec4 color;
    ivec4 meshSize;
} ubo;

void main(void)
{
    // Code

    //* Rebuild u and v from the vertex index
    int i = gl_VertexIndex % ubo.meshSize.x;
    int j = gl_VertexIndex / ubo.meshSize.y;

    float u = float(i) / float(ubo.meshSize.x) * 2.0 - 1.0;
    float v = float(j) / float(ubo.meshSize.y) * 2.0 - 1.0;

    gl_Position = ubo.mvpMatrix * vec4(u, vHeight, v, 1.0);
    gl_PointSize = 1.0;
}
