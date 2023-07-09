#include "ShaderStructs.hlsli"

float4 main(VertexToPixel input) : SV_TARGET
{
    //out_col = light.diffuse + light.ambient;
    //out_col *= saturate(dot(light.dir, input.nor));
    //out_col += saturate(dot(light.dir, input.nor)) * light.diffuse * diffuse;
    //out_col.a = diffuse.a;
    ////output_col.r = tex.x;
    ////output_col.g = tex.y;
    ////output_col.b = 0;
    ////output_col.a = 1;
    //return out_col;// float4(out_col, diffuse.a);

    float3 dir = {0.f, 1.f, 0.f };
    Light light;
    light.dir = normalize(dir);
    light.ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);
    light.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);

    float4 diffuse = input.col;
    float lightIntensity;
    float4 color;

    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.nor, light.dir));

    // Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
    color = saturate(light.diffuse * lightIntensity);

    // Multiply the texture pixel and the final diffuse color to get the final pixel color result.
    color = color * diffuse;

    return color;
}

