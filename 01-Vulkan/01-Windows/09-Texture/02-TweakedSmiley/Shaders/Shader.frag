#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 out_texcoord;

layout(binding = 0) uniform mvpMatrix 
{ 
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    int keyPressed;
} uMVP;


layout(binding = 1) uniform sampler2D utextureSampler;

layout(location = 0) out vec4 FragColor;

void main(void)
{
    // Code
    if (uMVP.keyPressed == 1)
        FragColor = texture(utextureSampler, out_texcoord);
    else
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
