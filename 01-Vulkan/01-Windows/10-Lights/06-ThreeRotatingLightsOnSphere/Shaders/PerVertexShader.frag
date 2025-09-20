#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 phong_ads_light;

layout(location = 0) out vec4 FragColor;

void main(void)
{
    // Code
    FragColor = vec4(phong_ads_light, 1.0);
}
