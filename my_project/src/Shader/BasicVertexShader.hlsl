struct Output
{
	float4 pos : POSITION;
	float4 sv_pos : SV_POSITION;
};

Output basicVS(float4 pos : POSITION)
{
	Output output;
	output.pos = pos;
	output.sv_pos = pos;
	return output;
}

/*
float4 basicVS(float4 pos : POSITION) : SV_POSITION
{
	return pos;
}
*/
