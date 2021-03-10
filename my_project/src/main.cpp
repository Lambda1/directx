#include "MyDirect3D12/MyDirect3D12.hpp"
#include "Vertex/Vertex.hpp"

#include <Windows.h>

#include <iostream>

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);

	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// �E�B���h�E�N���X������
HWND InitializeWindowClass(WNDCLASSEX *wnd_class_ex, const LPCWSTR &app_name, const LPCWSTR &title_name, const int &window_width, const int &window_height)
{
	// �E�B���h�E�N���X�̓o�^
	wnd_class_ex->cbSize = sizeof(WNDCLASSEX);
	wnd_class_ex->lpfnWndProc = reinterpret_cast<WNDPROC>(WindowProcedure);
	wnd_class_ex->lpszClassName = app_name;
	wnd_class_ex->hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(wnd_class_ex);

	// �E�B���h�E�T�C�Y�̕␳
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow
	(
		wnd_class_ex->lpszClassName,
		title_name,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wnd_class_ex->hInstance,
		nullptr
	);

	return hwnd;
}
// �E�B���h�E�N���X�̔j��
void TerminateWindowClass(WNDCLASSEX* wnd_class_ex)
{
	// �E�B���h�E�N���X�̓o�^����
	if (wnd_class_ex->lpszClassName) { UnregisterClass(wnd_class_ex->lpszClassName, wnd_class_ex->hInstance); }
}

int WINAPI WinMain(_In_ HINSTANCE h_instance, _In_opt_  HINSTANCE h_prev_instance, _In_ LPSTR lp_cmd_line, _In_ int n_show_cmd)
{
	// �E�B���h�E�ݒ�
	const LPCWSTR app_name = L"DirectX12";
	const LPCWSTR title_name = L"MyDirectX12";
	const int window_width = 1280;
	const int window_height = 720;
	// �E�B���h�E�I�u�W�F�N�g�̎擾
	WNDCLASSEX wnd_class_ex = {};
	HWND hwnd = InitializeWindowClass(&wnd_class_ex, app_name, title_name, window_height, window_height);
	// �E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	// D3D12������
	mla::MyDirect3D12 my_d3d{hwnd, window_width, window_height, L"Intel"};

	// �e�N�X�`���f�[�^
	struct TexRGBA
	{
		unsigned char r, g, b, a;
	};
	std::vector<TexRGBA> texture_data(256 * 256);
	for (auto data : texture_data)
	{
		data.r = rand() % 256;
		data.g = rand() % 256;
		data.b = rand() % 256;
		data.a = 255;
	}
	// �e�N�X�`���o�b�t�@�쐬
	D3D12_HEAP_PROPERTIES tex_heap_prop = {};
	tex_heap_prop.Type = D3D12_HEAP_TYPE_CUSTOM;
	tex_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	tex_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	tex_heap_prop.CreationNodeMask = 0;
	tex_heap_prop.VisibleNodeMask = 0;
	D3D12_RESOURCE_DESC tex_resc_desc = {};
	tex_resc_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_resc_desc.Width = 256;
	tex_resc_desc.Height = 256;
	tex_resc_desc.DepthOrArraySize = 1;
	tex_resc_desc.SampleDesc.Count = 1;
	tex_resc_desc.SampleDesc.Quality = 0;
	tex_resc_desc.MipLevels = 1;
	tex_resc_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	tex_resc_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	tex_resc_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	auto tex_buff = my_d3d.CreateCommitedResource(tex_heap_prop, tex_resc_desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	// �e�N�X�`���]��
	HRESULT tex_result = tex_buff->WriteToSubresource(0, nullptr, texture_data.data(), sizeof(TexRGBA) * 256, sizeof(TexRGBA) * texture_data.size());

	// �V�F�[�_�p�f�B�X�N���v�^�q�[�v
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> tex_desc_heap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc = {};
	desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc_heap_desc.NodeMask = 0;
	desc_heap_desc.NumDescriptors = 1;
	desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HRESULT shader_desc_result = my_d3d.GetDevice()->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(tex_desc_heap.ReleaseAndGetAddressOf()));
	// �V�F�[�_���\�[�X�r���[�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;
	my_d3d.GetDevice()->CreateShaderResourceView(tex_buff.Get(), &srv_desc, tex_desc_heap-> GetCPUDescriptorHandleForHeapStart());
	// �T���v��
	D3D12_STATIC_SAMPLER_DESC sampler_desc = {};
	sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler_desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
	sampler_desc.MinLOD = 0.0f;
	sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	// ���_�f�[�^
	mla::Vertex vertices[] =
	{
		{{-0.40f, -0.70f, 0.0f}, {0.0f, 1.0f}},
		{{-0.40f,  0.70f, 0.0f}, {0.0f, 0.0f}},
		{{ 0.40f, -0.70f, 0.0f}, {1.0f, 1.0f}},
		{{ 0.40f,  0.70f, 0.0f}, {1.0f, 0.0f}},
	};
	D3D12_HEAP_PROPERTIES heap_prop = {};
	heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	D3D12_RESOURCE_DESC resc_desc = {};
	resc_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resc_desc.Width = sizeof(vertices);
	resc_desc.Height = 1;
	resc_desc.DepthOrArraySize = 1;
	resc_desc.MipLevels = 1;
	resc_desc.Format = DXGI_FORMAT_UNKNOWN;
	resc_desc.SampleDesc.Count = 1;
	resc_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resc_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	auto vert_buff = my_d3d.CreateCommitedResource(heap_prop, resc_desc, D3D12_RESOURCE_STATE_GENERIC_READ);
	// ���_�f�[�^�}�b�s���O
	my_d3d.Mapping<mla::Vertex>(vertices, sizeof(vertices), vert_buff);
	// ���_�o�b�t�@�r���[�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vb_view = {};
	vb_view.BufferLocation = vert_buff->GetGPUVirtualAddress();
	vb_view.SizeInBytes = sizeof(vertices);
	vb_view.StrideInBytes = sizeof(vertices[0]);
	
	// ���_�C���f�b�N�X
	unsigned short indices[] =
	{
		0, 1, 2,
		2, 1, 3
	};
	resc_desc.Width = sizeof(indices);
	auto ind_buff = my_d3d.CreateCommitedResource(heap_prop, resc_desc, D3D12_RESOURCE_STATE_GENERIC_READ);
	// �}�b�s���O
	my_d3d.Mapping<unsigned short>(indices, sizeof(indices), ind_buff);
	// �C���f�b�N�X�o�b�t�@�r���[�쐬
	D3D12_INDEX_BUFFER_VIEW ib_view = {};
	ib_view.BufferLocation = ind_buff->GetGPUVirtualAddress();
	ib_view.SizeInBytes = sizeof(indices);
	ib_view.Format = DXGI_FORMAT_R16_UINT;

	// �f�B�X�N���v�^�����W
	D3D12_DESCRIPTOR_RANGE desc_tbl_range = {};
	desc_tbl_range.NumDescriptors = 1;
	desc_tbl_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	desc_tbl_range.BaseShaderRegister = 0;
	desc_tbl_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ���[�g�p�����[�^
	D3D12_ROOT_PARAMETER root_param = {};
	root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	root_param.DescriptorTable.pDescriptorRanges = &desc_tbl_range;
	root_param.DescriptorTable.NumDescriptorRanges = 1;
	// ���[�g�V�O�l�`���ݒ�
	Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	root_signature_desc.pParameters = &root_param;
	root_signature_desc.NumParameters = 1;
	root_signature_desc.pStaticSamplers = &sampler_desc;
	root_signature_desc.NumStaticSamplers = 1;
	// �o�C�i���R�[�h�쐬
	Microsoft::WRL::ComPtr<ID3DBlob> error_blob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> root_sig_blob = nullptr;
	HRESULT root_sig_result = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, root_sig_blob.ReleaseAndGetAddressOf(), error_blob.ReleaseAndGetAddressOf());
	if (root_sig_result != S_OK) { my_d3d.ErrorBlob(error_blob); }
	// ���[�g�V�O�l�`���쐬
	root_sig_result = my_d3d.GetDevice()->CreateRootSignature(0, root_sig_blob->GetBufferPointer(), root_sig_blob->GetBufferSize(), IID_PPV_ARGS(root_signature.ReleaseAndGetAddressOf()));
	if (root_sig_result != S_OK) { std::exit(EXIT_FAILURE); }

	// �p�C�v���C���X�e�[�g�ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC g_pipeline = {};
	g_pipeline.pRootSignature = root_signature.Get();
	// �V�F�[�_�R���p�C��
	my_d3d.CompileBasicShader(L"./src/Shader/BasicVertexShader.hlsl", L"./src/Shader/BasicPixelShader.hlsl", &g_pipeline);

	// ���C������
	MSG msg = {};
	while (true)
	{
		my_d3d.BeginDraw();
		FLOAT col[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		my_d3d.ClearRenderTarget(col);
		// �p�C�v���C���X�e�[�g
		my_d3d.SetPipelineState();
		// ���[�g�V�O�l�`��
		my_d3d.GetCommandList()->SetGraphicsRootSignature(root_signature.Get());
		// �f�B�X�N���v�^�q�[�v
		my_d3d.GetCommandList()->SetDescriptorHeaps(1, tex_desc_heap.GetAddressOf());
		// �f�B�X�N���v�^�q�[�v�֘A�t��
		my_d3d.GetCommandList()->SetGraphicsRootDescriptorTable(0, tex_desc_heap->GetGPUDescriptorHandleForHeapStart());
		// �̈�
		my_d3d.SetViewAndScissor();
		// �g�|���W
		my_d3d.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// ���_�o�b�t�@
		my_d3d.GetCommandList()->IASetVertexBuffers(0, 1, &vb_view);
		// �C���f�b�N�X�o�b�t�@
		my_d3d.GetCommandList()->IASetIndexBuffer(&ib_view);
		// �`��
		my_d3d.GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
		my_d3d.EndDraw();

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) { break; }
	}

	// �E�B���h�E�I�u�W�F�N�g�̉���
	TerminateWindowClass(&wnd_class_ex);

	return 0;
}