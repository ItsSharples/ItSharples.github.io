#include "ShaderStructs.hlsli"

TexturedVertexToPixel main(TexturedAppToVertex input)
{
	TexturedVertexToPixel output;


	output.pos = input.pos;
	output.pos = output.pos + coord_pos;
	output.col = input.col;


	output.pos.w = 1.0f;
	// Move from 3D position onto the 2D Screen
	//output.pos = mul(output.pos, worldMatrix);
	output.pos = mul(output.pos, viewMatrix);
	output.pos = mul(output.pos, projectionMatrix);

	output.tex = input.tex;
	output.nor = input.nor;

	return output;
}