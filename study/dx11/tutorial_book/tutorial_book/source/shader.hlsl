// グローバル変数
cbuffer global
{
	// World * View * Projection 行列
	matrix g_wvp;
};

// VertexShader
float4 VertexS(float4 Pos : POSITION) : SV_POSITION
{
	Pos = mul(Pos, g_wvp);
	return Pos;
}

// PixelShader
float4 PixelS(float4 Pos : SV_POSITION) : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}