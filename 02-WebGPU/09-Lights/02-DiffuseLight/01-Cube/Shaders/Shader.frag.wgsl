struct HostUniform
{
    modelMatrix: mat4x4<f32>,
    viewMatrix: mat4x4<f32>,
    projectionMatrix: mat4x4<f32>,
    lightAmbient: vec4<f32>,
    lightDiffuse: vec4<f32>,
    lightSpecular: vec4<f32>,
    lightPosition: vec4<f32>,
    materialAmbient: vec4<f32>,
    materialDiffuse: vec4<f32>,
    materialSpecular: vec4<f32>,
    materialShininess: vec4<f32>,
    lightEnabled: vec4<u32>
};

struct VertexOutput
{
    @builtin(position) position : vec4<f32>,
    @location(0) diffused_light_color : vec3<f32>
};

@fragment
fn main(output : VertexOutput) -> @location(0) vec4<f32>
{
    var color = vec4(output.diffused_light_color, 1.0);
    return color;
}
