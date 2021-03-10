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
	mla::MyDirect3D12 my_d3d{hwnd, window_width, window_height, L"NVIDIA"};

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
	auto vert_buff = my_d3d.CreateCommitedResource(heap_prop, resc_desc);
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
	auto ind_buff = my_d3d.CreateCommitedResource(heap_prop, resc_desc);
	// �}�b�s���O
	my_d3d.Mapping<unsigned short>(indices, sizeof(indices), ind_buff);
	// �C���f�b�N�X�o�b�t�@�r���[�쐬
	D3D12_INDEX_BUFFER_VIEW ib_view = {};
	ib_view.BufferLocation = ind_buff->GetGPUVirtualAddress();
	ib_view.SizeInBytes = sizeof(indices);
	ib_view.Format = DXGI_FORMAT_R16_UINT;

	// ���[�g�V�O�l�`���ݒ�
	Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
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