#include "SimpleTriangle.hpp"

/* public */

void my_lib::SimpleTriangle::Prepare()
{
	// ŽOŠpŒ`’è‹`
	Vertex triangle_vertices[] =
	{
		{ { 0.00f,  0.25f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
		{ { 0.25f, -0.25f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
		{ {-0.25f, -0.25f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
	};
	uint32_t indices[] = { 0, 1, 2 };

	// Compile Shader
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> error_blob;
	// Vertex Shader
	hr = CompileShaderFromFile(L"./simpleVS.hlsl", L"vs_6_0", m_vertex_shader, error_blob);
	if (FAILED(hr)) { OutputDebugStringA((const char*)error_blob->GetBufferPointer()); }
	// Pixel Shader
	hr = CompileShaderFromFile(L"./simplePS.hlsl", L"ps_6_0", m_pixel_shader, error_blob);
	if (FAILED(hr)) { OutputDebugStringA((const char*)error_blob->GetBufferPointer()); }
}

void my_lib::SimpleTriangle::CleanUp()
{

}

void my_lib::SimpleTriangle::MakeCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& command)
{

}
