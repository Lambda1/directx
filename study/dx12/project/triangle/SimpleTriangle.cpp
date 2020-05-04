#include "SimpleTriangle.hpp"

/* private */
Microsoft::WRL::ComPtr<ID3D12Resource1> my_lib::SimpleTriangle::CreateBuffer(UINT buffer_size, const void* initial_data)
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3D12Resource1> buffer;

	hr = m_device->CreateCommittedResource
	(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(buffer_size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&buffer)
	);

	// 初期データ指定がある時はコピー
	if (SUCCEEDED(hr) && !initial_data)
	{
		void* mapped;
		CD3DX12_RANGE range(0, 0);
		hr = buffer->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, initial_data, buffer_size);
			buffer->Unmap(0, nullptr);
		}
	}

	return buffer;
}

/* public */

void my_lib::SimpleTriangle::Prepare()
{
	HRESULT hr;

	// 三角形定義
	Vertex triangle_vertices[] =
	{
		{ { 0.00f,  0.25f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
		{ { 0.25f, -0.25f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
		{ {-0.25f, -0.25f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
	};
	uint32_t indices[] = { 0, 1, 2 };

	// Vertex, Index buffer生成
	m_vertex_buffer = CreateBuffer(sizeof(triangle_vertices), triangle_vertices);
	m_index_buffer = CreateBuffer(sizeof(indices), indices);
	m_index_count = _countof(indices);

	// 各buffer view生成
	m_vertex_buffer_view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
	m_vertex_buffer_view.SizeInBytes = sizeof(triangle_vertices);
	m_vertex_buffer_view.StrideInBytes = sizeof(Vertex);
	m_index_buffer_view.BufferLocation = m_index_buffer->GetGPUVirtualAddress();
	m_index_buffer_view.SizeInBytes = sizeof(indices);
	m_index_buffer_view.Format = DXGI_FORMAT_R32_UINT;

	// Compile Shader
	Microsoft::WRL::ComPtr<ID3DBlob> error_blob;
	UINT compile_flags = 0;
	// Vertex Shader
	hr = CompileShaderFromFile(L"./simpleVS.hlsl", L"vs_6_0", m_vertex_shader, error_blob);
	if (FAILED(hr)) { OutputDebugStringA((const char*)error_blob->GetBufferPointer()); }
	// Pixel Shader
	hr = CompileShaderFromFile(L"./simplePS.hlsl", L"ps_6_0", m_pixel_shader, error_blob);
	if (FAILED(hr)) { OutputDebugStringA((const char*)error_blob->GetBufferPointer()); }

	// Root signature構築
	CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc{};
	root_signature_desc.Init
	(
		0, nullptr,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	D3D12SerializeRootSignature
	(
		&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&signature, &error
	);
	// Root signature生成
	hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_root_signature));
	if (FAILED(hr)) { throw std::runtime_error("CreateRootSignature failed."); }

	// Input Layout : 三角形頂点入力設定
	D3D12_INPUT_ELEMENT_DESC input_element_desc[] =
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, offsetof(Vertex, m_position),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
		},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
			0, offsetof(Vertex, m_color),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
		}
	};

	// Pipeline State Object生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
	// Shader設定
	pso_desc.VS = CD3DX12_SHADER_BYTECODE(m_vertex_shader.Get());
	pso_desc.PS = CD3DX12_SHADER_BYTECODE(m_pixel_shader.Get());
	// Blend state設定
	pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	// Rasterizer state設定
	pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	// 出力先は1ターゲット
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	// Depth bufferフォーマット設定
	pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pso_desc.InputLayout = { input_element_desc, _countof(input_element_desc) };
	pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	// Root signature設定
	pso_desc.pRootSignature = m_root_signature.Get();
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// Multi sample設定
	pso_desc.SampleDesc = { 1, 0 };
	pso_desc.SampleMask = UINT_MAX;

	// PSO生成
	hr = m_device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_pipeline));
	if (FAILED(hr)) { throw std::runtime_error("CreateFraphicsPipelineState failed."); }
}

void my_lib::SimpleTriangle::CleanUp()
{

}

void my_lib::SimpleTriangle::MakeCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& command)
{
	// Pipeline state設定
	command->SetPipelineState(m_pipeline.Get());
	// Root signature設定
	command->SetGraphicsRootSignature(m_root_signature.Get());
	// viewportとscissor設定
	command->RSSetViewports(1, &m_viewport);
	command->RSSetScissorRects(1, &m_scissor_rect);

	// Primitive, Vertex, Index Buffer設定
	command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	command->IASetVertexBuffers(0, 1, &m_vertex_buffer_view);
	command->IASetIndexBuffer(&m_index_buffer_view);

	// 描画命令発行
	m_command_list->DrawIndexedInstanced(3, 1, 0, 0, 0);
}
