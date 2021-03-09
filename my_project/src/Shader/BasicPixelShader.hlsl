struct Input
{
	float4 pos : POSITION;
	float4 sv_pos : SV_POSITION;
};

float4 basicPS(Input input) : SV_TARGET
{
	return float4((float2(0, 1) + input.pos.xy) * 0.5f, 1, 1);
}

/*
float4 basicPS(float4 pos : SV_POSITION) : SV_TARGET
{
	return float4(pos.x/1280.0f, pos.y/720.0f, 1, 1);
}
*/