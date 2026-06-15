#version 460 core

layout(triangles, equal_spacing, cw) in;

layout(binding = 0) uniform uniformData
{
    mat4 mvpMatrix;
    vec4 numberOfLineSegments;
    vec4 numberOfStrips;
    vec4 lineColor;
} ubo;

vec3 cubicBezier(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t)
{
    float u = 1.0 - t;

    return
        u*u*u * p0 +
        3.0*u*u*t * p1 +
        3.0*u*t*t * p2 +
        t*t*t * p3;
}

void main()
{
    float t = gl_TessCoord.x;

    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 p3 = gl_in[3].gl_Position.xyz;

    vec3 pos = cubicBezier(p0, p1, p2, p3, t);

    gl_Position = ubo.mvpMatrix * vec4(pos, 1.0);
}
