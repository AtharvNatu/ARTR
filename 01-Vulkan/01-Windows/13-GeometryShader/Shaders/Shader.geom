#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;

layout(triangle_strip, max_vertices = 9) out;

layout(binding = 0) uniform uniformData
{ 
    mat4 mvpMatrix;
} ubo;

void main(void)
{
    for (int i = 0; i < 3; i++)
    {
        gl_Position = ubo.mvpMatrix * (gl_in[i].gl_Position + vec4(0.0, 1.0, 0.0, 1.0));
        EmitVertex();

        gl_Position = ubo.mvpMatrix * (gl_in[i].gl_Position + vec4(-1.0, -1.0, 0.0, 1.0));
        EmitVertex();

        gl_Position = ubo.mvpMatrix * (gl_in[i].gl_Position + vec4(1.0, -1.0, 0.0, 1.0));
        EmitVertex();
        
        EndPrimitive();
    } 
}



        