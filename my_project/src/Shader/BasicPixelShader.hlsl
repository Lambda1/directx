#include "./BasicShaderHeader.hlsli"

float4 basicPS(Output input) : SV_TARGET
{
	return float4(input.uv, 1, 1);
}
