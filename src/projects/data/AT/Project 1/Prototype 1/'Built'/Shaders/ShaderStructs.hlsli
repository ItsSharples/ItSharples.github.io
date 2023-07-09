// Globals
#include "buffers.hlsli"

Texture2D shaderTexture;
SamplerState SampleType;

SamplerState DefaultSamplerState
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
	BorderColor = { 1, 1, 1, 1 };
};

// Structures
struct Light
{
    float3 dir;
    float4 ambient;
    float4 diffuse;
};

struct AppToVertex
{
	float4 pos : POSITION;	
	float4 col : COLOR;
	float4 nor : NORMAL;
};

struct VertexToPixel
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float4 nor : NORMAL;
};

struct TexturedAppToVertex
{
	float4 pos : POSITION;
	float4 col : COLOR;
	float4 nor : NORMAL;
	float2 tex : TEXCOORD0;

};

struct TexturedVertexToPixel
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float4 nor : NORMAL;
	float2 tex : TEXCOORD0;
};



struct SkyboxVertexToPixel
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float4 nor : NORMAL;
	float2 tex : TEXCOORD0;
};