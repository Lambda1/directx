#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1.0f, -1.0f, 1.0f));
	float brightness = dot(-light, input.normal);

	return float4(brightness, brightness, brightness, 1.0f) * diffuse;
}