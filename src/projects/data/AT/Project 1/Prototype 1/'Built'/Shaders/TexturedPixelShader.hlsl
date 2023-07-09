#include "ShaderStructs.hlsli"


float4 main(TexturedVertexToPixel input): SV_TARGET
{
    Light light;
    light.dir = (-1.f, 2.f, 0.0f);
    light.ambient = (0.1f, 0.1f, 0.1f, 1.0f);
    light.diffuse = (1.0f, 1.0f, 1.0f, 1.0f); 

    float3 out_col;
    float2 tex = input.tex.xy;

    float4 diffuse = shaderTexture.Sample(DefaultSamplerState, tex);

    out_col = diffuse * light.ambient;
    out_col += saturate(dot(light.dir, input.nor) * light.diffuse * diffuse);

    //output_col.r = tex.x;
    //output_col.g = tex.y;
    //output_col.b = 0;
    //output_col.a = 1;

    return float4(out_col, diffuse.a);
}