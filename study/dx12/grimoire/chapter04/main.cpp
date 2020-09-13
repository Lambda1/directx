#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <iostream>
#include <vector>
#include <string>

// �O���[�o���ϐ�
const int window_width = 800;
const int window_height = 600;
// D3D12
ID3D12Device* p_device = nullptr;
IDXGIFactory6* p_dxgi_factory = nullptr;
IDXGISwapChain4* p_swap_chain = nullptr;
ID3D12CommandAllocator* p_cmd_allocator = nullptr;
ID3D12GraphicsCommandList* p_cmd_list = nullptr;
ID3D12CommandQueue* p_cmd_queue = nullptr;
std::vector<ID3D12Resource*> back_buffers;
ID3D12DescriptorHeap* rtv_heaps = nullptr;
ID3D12Fence* p_fence = nullptr;
UINT64 fence_val = 0;
ID3DBlob* p_vs_blob = nullptr;
ID3DBlob* p_ps_blob = nullptr;
ID3D12PipelineState* p_pipeline_state = nullptr;

// WinAPI
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E�j������
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// �f�o�b�O�p�֐�
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	std::cout << format << " " << valist << std::endl;
	va_end(valist);
#endif
}

// DirectX12
void InitDirectX(HWND& hwnd)
{
	// �A�_�v�^��
#ifdef _DEBUG
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&p_dxgi_factory))))
#else
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&p_dxgi_factory))))
#endif
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* p_tmp_adapter = nullptr;
	for (int i = 0; p_dxgi_factory->EnumAdapters(i, &p_tmp_adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.emplace_back(p_tmp_adapter);
	}
	// �A�_�v�^����
	IDXGIAdapter* p_adapter = nullptr;
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring str_desc = adesc.Description;
		std::wcout << str_desc << std::endl;
		if (str_desc.find(L"Intel") != std::string::npos)
		{
			if (!p_adapter) { p_adapter = adpt; }
		}
	}

	// �f�o�C�X�I�u�W�F�N�g����
	D3D_FEATURE_LEVEL level;
	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_1 };
	for (auto lv : levels)
	{
		if (SUCCEEDED(D3D12CreateDevice(p_adapter, lv, IID_PPV_ARGS(&p_device))))
		{
			level = lv;
			break;
		}
	}
	if (!p_device)
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// �R�}���h�A���P�[�^����
	if (FAILED(p_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&p_cmd_allocator))))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}
	// �R�}���h���X�g����
	if (FAILED(p_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p_cmd_allocator, nullptr, IID_PPV_ARGS(&p_cmd_list))))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// �R�}���h�L���[����
	D3D12_COMMAND_QUEUE_DESC cmd_queu_desc = {};
	cmd_queu_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // �^�C���A�E�g����
	cmd_queu_desc.NodeMask = 0; // �A�_�v�^1�̏ꍇ��0
	cmd_queu_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmd_queu_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // �R�}���h���X�g�ɍ��킹��
	if (FAILED(p_device->CreateCommandQueue(&cmd_queu_desc, IID_PPV_ARGS(&p_cmd_queue))))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// �X���b�v�`�F�[������
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width = window_width;
	swap_chain_desc.Height = window_height;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = false;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swap_chain_desc.BufferCount = 2;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH; // �o�b�N�o�b�t�@�͐L�k�\
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // �t���b�v��͔j��
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // �E�B���h�E�E�t���X�N�؂�ւ�
	if (FAILED(p_dxgi_factory->CreateSwapChainForHwnd(p_cmd_queue, hwnd, &swap_chain_desc, nullptr, nullptr, (IDXGISwapChain1**)&p_swap_chain)))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// �f�B�X�N���v�^�q�[�v����
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heap_desc.NodeMask = 0;
	heap_desc.NumDescriptors = 2;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(p_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&rtv_heaps))))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}

	// �X���b�v�`�F�[���ƃ������̕R�Â�
	DXGI_SWAP_CHAIN_DESC swc_desc = {};
	if (FAILED(p_swap_chain->GetDesc(&swc_desc)))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}
	back_buffers.resize(swc_desc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_heaps->GetCPUDescriptorHandleForHeapStart();
	for (int index = 0; index < swc_desc.BufferCount; ++index)
	{
		if (FAILED(p_swap_chain->GetBuffer(index, IID_PPV_ARGS(&back_buffers[index]))))
		{
			std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
		}
		handle.ptr += (index * p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		p_device->CreateRenderTargetView(back_buffers[index], nullptr, handle);
	}

	// �t�F���X����
	if (FAILED(p_device->CreateFence(fence_val, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&p_fence))))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}
}

void EnableDebugLayer()
{
	ID3D12Debug* debug_layer = nullptr;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_layer));
	if (FAILED(hr))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}

	debug_layer->EnableDebugLayer();
	debug_layer->Release();
}

#ifdef _DEBUG
int main()
#else
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
int main()
#endif
{
	// �E�B���h�E�N���X�̐����E�o�^
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = TEXT("DX12 Sample");
	w.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&w);

	// �E�B���h�E�T�C�Y�̒���
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow
	(
		w.lpszClassName,
		TEXT("DX12 �e�X�g"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr
	);

#ifdef _DEBUG
	DebugOutputFormatString("Show window test.");
#endif

	// D3D12�̏�����
#ifdef _DEBUG
	EnableDebugLayer();
#endif
	InitDirectX(hwnd);

	// �E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	// ���_�f�[�^
	DirectX::XMFLOAT3 vertices[] =
	{
		{-0.4f, -0.7f, 0.0f},
		{-0.4f,  0.7f, 0.0f},
		{ 0.4f, -0.7f, 0.0f},
		{ 0.4f,  0.7f, 0.0f}
	};
	// �C���f�b�N�X�f�[�^
	unsigned short indices[] =
	{
		0, 1, 2,
		2, 1, 3
	};

	// ���_�o�b�t�@����
	D3D12_HEAP_PROPERTIES heap_prop = {};
	heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC res_desc = {};
	res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	res_desc.Width = sizeof(vertices);
	res_desc.Height = 1;
	res_desc.DepthOrArraySize = 1;
	res_desc.MipLevels = 1;
	res_desc.Format = DXGI_FORMAT_UNKNOWN;
	res_desc.SampleDesc.Count = 1;
	res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vert_buff = nullptr;
	if (FAILED(p_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vert_buff))))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}

	// ���_�f�[�^�̃}�b�v
	DirectX::XMFLOAT3* vert_map = nullptr;
	if (FAILED(vert_buff->Map(0, nullptr, (void**)&vert_map)))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}
	std::copy(std::begin(vertices), std::end(vertices), vert_map);
	vert_buff->Unmap(0, nullptr);
	// ���_�o�b�t�@�r���[�쐬
	D3D12_VERTEX_BUFFER_VIEW vb_view = {};
	vb_view.BufferLocation = vert_buff->GetGPUVirtualAddress();
	vb_view.SizeInBytes = sizeof(vertices);
	vb_view.StrideInBytes = sizeof(vertices[0]);
	// �C���f�b�N�X�o�b�t�@����
	ID3D12Resource* idx_buffer = nullptr;
	res_desc.Width = sizeof(indices);
	if (FAILED(p_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idx_buffer))))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}
	// �C���f�b�N�X�f�[�^�̃}�b�v
	unsigned short* mapped_idx = nullptr;
	idx_buffer->Map(0, nullptr, (void**)&mapped_idx);
	std::copy(std::begin(indices), std::end(indices), mapped_idx);
	idx_buffer->Unmap(0, nullptr);
	// �C���f�b�N�X�o�b�t�@�r���[�쐬
	D3D12_INDEX_BUFFER_VIEW ib_view = {};
	ib_view.BufferLocation = idx_buffer->GetGPUVirtualAddress();
	ib_view.Format = DXGI_FORMAT_R16_UINT;
	ib_view.SizeInBytes = sizeof(indices);

	// �V�F�[�_�Ǘ�
	ID3DBlob* error_blob = nullptr;
	auto shader_error_func = [&]()
	{
		std::string error_str;
		error_str.resize(error_blob->GetBufferSize());
		std::copy_n((char*)error_blob->GetBufferPointer(), error_blob->GetBufferSize(), error_str.begin());
		error_str += '\n';
		OutputDebugStringA(error_str.c_str());
	};
	if (FAILED(D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &p_vs_blob, &error_blob)))
	{
		shader_error_func();
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}
	if (FAILED(D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &p_ps_blob, &error_blob)))
	{
		shader_error_func();
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}

	// ���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC input_layout[] =
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// �V�F�[�_�̃Z�b�g
	D3D12_GRAPHICS_PIPELINE_STATE_DESC g_pipeline_desc = {};
	g_pipeline_desc.pRootSignature = nullptr;
	g_pipeline_desc.VS.pShaderBytecode = p_vs_blob->GetBufferPointer();
	g_pipeline_desc.VS.BytecodeLength = p_vs_blob->GetBufferSize();
	g_pipeline_desc.PS.pShaderBytecode = p_ps_blob->GetBufferPointer();
	g_pipeline_desc.PS.BytecodeLength = p_ps_blob->GetBufferSize();

	// �T���v���}�X�g�E���X�^���C�U�X�e�[�g�̐ݒ�
	g_pipeline_desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	g_pipeline_desc.RasterizerState.MultisampleEnable = false;
	g_pipeline_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	g_pipeline_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	g_pipeline_desc.RasterizerState.DepthClipEnable = true;

	// ������Ă��Ȃ��������
	g_pipeline_desc.RasterizerState.FrontCounterClockwise = false;
	g_pipeline_desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	g_pipeline_desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	g_pipeline_desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	g_pipeline_desc.RasterizerState.AntialiasedLineEnable = false;
	g_pipeline_desc.RasterizerState.ForcedSampleCount = 0;
	g_pipeline_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	g_pipeline_desc.DepthStencilState.DepthEnable = false;
	g_pipeline_desc.DepthStencilState.StencilEnable = false;

	// �u�����h�X�e�[�g�ݒ�
	g_pipeline_desc.BlendState.AlphaToCoverageEnable = false;
	g_pipeline_desc.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC render_target_blend_desc = {};
	render_target_blend_desc.BlendEnable = false;
	render_target_blend_desc.LogicOpEnable = false;
	render_target_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	g_pipeline_desc.BlendState.RenderTarget[0] = render_target_blend_desc;

	// ���̓��C�A�E�g�ݒ�
	g_pipeline_desc.InputLayout.pInputElementDescs = input_layout;
	g_pipeline_desc.InputLayout.NumElements = _countof(input_layout);

	g_pipeline_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	g_pipeline_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// �����_�[�^�[�Q�b�g�ݒ�
	g_pipeline_desc.NumRenderTargets = 1;
	g_pipeline_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	// �A���`�G�C���A�V���O�ݒ�
	g_pipeline_desc.SampleDesc.Count = 1;
	g_pipeline_desc.SampleDesc.Quality = 0;

	// ���[�g�V�O�l�`���̍쐬
	ID3D12RootSignature* p_root_signature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* root_signature_blob = nullptr;
	if (FAILED(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &root_signature_blob, &error_blob)))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}

	if (FAILED(p_device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&p_root_signature))))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}
	root_signature_blob->Release();

	g_pipeline_desc.pRootSignature = p_root_signature;

	// �O���t�B�b�N�p�C�v���C���X�e�[�g�I�u�W�F�N�g����
	if (FAILED(p_device->CreateGraphicsPipelineState(&g_pipeline_desc, IID_PPV_ARGS(&p_pipeline_state))))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}

	// �r���[�|�[�g�ݒ�
	D3D12_VIEWPORT view_port = {};
	view_port.Width = window_width;
	view_port.Height = window_height;
	view_port.TopLeftX = 0;
	view_port.TopLeftY = 0;
	view_port.MaxDepth = 1.0f;
	view_port.MinDepth = 0.0f;

	// �V�U�[��`
	D3D12_RECT scissor_rect = {};
	scissor_rect.top = 0;
	scissor_rect.left = 0;
	scissor_rect.right = scissor_rect.left + window_width;
	scissor_rect.bottom = scissor_rect.top + window_height;

	// ���[�v
	HRESULT hr = FALSE;
	MSG msg = {};
	while (true)
	{
		// WinAPI
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}

		// D3D12
		// �R�}���h�N���A
		UINT bb_idx = p_swap_chain->GetCurrentBackBufferIndex();

		// �o���A�ݒ�
		D3D12_RESOURCE_BARRIER barrier_desc = {};
		barrier_desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_desc.Transition.pResource = back_buffers[bb_idx];
		barrier_desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier_desc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier_desc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		p_cmd_list->ResourceBarrier(1, &barrier_desc);
		
		p_cmd_list->SetPipelineState(p_pipeline_state);

		// RT�ݒ�
		auto rtv_h = rtv_heaps->GetCPUDescriptorHandleForHeapStart();
		rtv_h.ptr += bb_idx * p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		p_cmd_list->OMSetRenderTargets(1, &rtv_h, false, nullptr);

		// RT�N���A
		float clear_color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		p_cmd_list->ClearRenderTargetView(rtv_h, clear_color, 0, nullptr);

		// �`�施��
		p_cmd_list->RSSetViewports(1, &view_port);
		p_cmd_list->RSSetScissorRects(1, &scissor_rect);
		p_cmd_list->SetGraphicsRootSignature(p_root_signature);
		p_cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		p_cmd_list->IASetVertexBuffers(0, 1, &vb_view);
		p_cmd_list->IASetIndexBuffer(&ib_view);
		
		p_cmd_list->DrawIndexedInstanced(6, 1, 0, 0, 0);

		barrier_desc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier_desc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		p_cmd_list->ResourceBarrier(1, &barrier_desc);

		// �N���[�Y
		p_cmd_list->Close();

		// �R�}���h���X�g���s
		ID3D12CommandList* cmd_lists[] = { p_cmd_list };
		p_cmd_queue->ExecuteCommandLists(1, cmd_lists);

		// �t�F���X����
		p_cmd_queue->Signal(p_fence, ++fence_val);
		if (p_fence->GetCompletedValue() != fence_val)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr); // �C�x���g�n���h���擾
			p_fence->SetEventOnCompletion(fence_val, event);

			WaitForSingleObject(event, INFINITE); // �C�x���g�I���҂�

			CloseHandle(event);
		}

		// ���
		p_cmd_allocator->Reset();
		p_cmd_list->Reset(p_cmd_allocator, nullptr); // �ēx�R�}���h���X�g�����߂鏀��

		// �X���b�v
		p_swap_chain->Present(1, 0);
	}

	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}