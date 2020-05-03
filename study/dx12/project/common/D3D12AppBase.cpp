#include "D3D12AppBase.hpp"

my_lib::D3D12AppBase::D3D12AppBase()
{
	m_render_targets.resize(m_frame_buffer_count);
}

/* private */

// DebugLayer�ݒ�
void my_lib::D3D12AppBase::DebugMode(UINT* dxgi_flags)
{
	Microsoft::WRL::ComPtr<ID3D12Debug> debug;

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
	{
		// DebugLayer�L����
		debug->EnableDebugLayer();
		if (dxgi_flags) { *dxgi_flags |= DXGI_CREATE_FACTORY_DEBUG; }

		// GPUBaseValidation�L����
		// NOTE: �V�F�[�_�����̓p�t�H�[�}���X�ɏd��ȉe�����y�ڂ��̂�, �K�v�Ȏ��̂�GBV���g������.
#if (_DEBUG_GBV)
		Microsoft::WRL::ComPtr<ID3D12Debug3> debug3;
		HRESULT hr = debug.As(&debug3);
		if (debug3) { debug3->SetEnableGPUBasedValidation(true); }
#endif
	}
}

// �n�[�h�E�F�A�A�_�v�^�ݒ�
Microsoft::WRL::ComPtr<IDXGIAdapter1> my_lib::D3D12AppBase::SearchHardwareAdapter(const Microsoft::WRL::ComPtr<IDXGIFactory3>& factory)
{
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

	for (UINT adapter_index = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapter_index, &adapter); ++adapter_index)
	{
		DXGI_ADAPTER_DESC1 desc{};
		adapter->GetDesc1(&desc);
		// �n�[�h�E�F�A�f�o�C�X��I��
		// NOTE: �\�t�g�E�F�A��WARP���g�p
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { continue; }

		// D3D12CreateDevice: �g�p�A�_�v�^, �v���@�\���x��, �f�o�C�X�擾
		HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr)) { return adapter.Get(); }
	}
	// DirectX12�Ή��A�_�v�^�Ȃ�
	return nullptr;
}
// �\�t�g�E�F�A�f�o�C�X�ݒ�
void my_lib::D3D12AppBase::SetWARPDevice(const Microsoft::WRL::ComPtr<IDXGIFactory4>& factory, Microsoft::WRL::ComPtr<IDXGIAdapter>& warp_device)
{
	factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_device));
}

/* protected */

// �f�B�X�N���v�^�q�[�v�̏���
void my_lib::D3D12AppBase::PrepareDescriptorHeaps()
{
	HRESULT hr = FALSE;
	
	// RenderTargetView(RTV)�p�f�B�X�N���v�^�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_frame_buffer_count, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
	hr = m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&m_rtv_heap));
	if (FAILED(hr)) { throw std::runtime_error("RTV CreateDescriptorHeap is failed."); }
	m_rtv_descripter_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	// DepthStencilView(DSV)�p�f�B�X�N���v�^�q�[�v
	// NOTE: �f�v�X�o�b�t�@��, (����)1�ŏ\��.
	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
	hr = m_device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&m_dsv_heap));
	if (FAILED(hr)) { throw std::runtime_error("DSV CreateDescriptorHeap is failed."); }
}
// �����_�[�^�[�Q�b�g�r���[�̐���
void my_lib::D3D12AppBase::PrepareRenderTargetView()
{
	// �X���b�v�`�F�C���C���[�W�ւ�RTV����
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_heap->GetCPUDescriptorHandleForHeapStart());
	
	for (UINT i = 0; i < m_frame_buffer_count; ++i)
	{
		// i�Ԗڂ�RT�̃��\�[�X�擾
		m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_render_targets[i]));
		// RTV�p�f�B�X�N���v�^�쐬
		m_device->CreateRenderTargetView(m_render_targets[i].Get(), nullptr, rtv_handle);
		// ���̃f�B�X�N���v�^�ւ̃A�h���X�v�Z
		rtv_handle.Offset(1, m_rtv_descripter_size);
	}
}
// �f�v�X�o�b�t�@�֌W�̏���
void my_lib::D3D12AppBase::CreateDepthBuffer(const int& width, const int& height)
{
	// �f�v�X�o�b�t�@����
	// NOTE: RT�Ɠ����摜�T�C�Y��, �e�N�X�`����1��Ƃ��č쐬.
	CD3DX12_RESOURCE_DESC depth_buffer_desc = CD3DX12_RESOURCE_DESC::Tex2D
	(
		DXGI_FORMAT_D32_FLOAT,
		width, height,
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	D3D12_CLEAR_VALUE depth_clear_value{};
	depth_clear_value.Format = depth_buffer_desc.Format;
	depth_clear_value.DepthStencil.Depth = 1.0f;
	depth_clear_value.DepthStencil.Stencil = 0;
	// �f�v�X�o�b�t�@�쐬
	m_device->CreateCommittedResource
	(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depth_buffer_desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depth_clear_value,
		IID_PPV_ARGS(&m_depth_buffer)
	);

	// DSV����
	// D3D12_DEPTH_STENCIL_VIEW_DESC: format, view dimension, flags, D3D12_TEX2D_DSV(MipSlice)
	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{ DXGI_FORMAT_D32_FLOAT, D3D12_DSV_DIMENSION_TEXTURE2D, D3D12_DSV_FLAG_NONE, {0} };
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle(m_dsv_heap->GetCPUDescriptorHandleForHeapStart());
	m_device->CreateDepthStencilView(m_depth_buffer.Get(), &dsv_desc, dsv_handle);
}

/* public */

void my_lib::D3D12AppBase::Initialize(HWND hWnd)
{
	HRESULT hr = FALSE;
	UINT dxgi_flags = 0;

	// DebugLayer
#if defined(_DEBUG)
	DebugMode(&dxgi_flags);
#endif

	// DXGI Factory����
	Microsoft::WRL::ComPtr<IDXGIFactory3> factory;
	hr = CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(&factory));
	if (FAILED(hr)) { throw std::runtime_error("CreateDXGIFactory2 is failed."); }

	// D3D12�f�o�C�X����
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter = SearchHardwareAdapter(factory);
	Microsoft::WRL::ComPtr<IDXGIAdapter1> use_adapter;
	// �n�[�h�E�F�A�A�_�v�^�ݒ�
	if (adapter.Get()) { adapter.As(&use_adapter); }
	// WARP�ݒ�
	// TODO: �n�[�h�E�F�A�A�_�v�^���Ȃ��ꍇ, WARP��ݒ�
	else { std::cout << "DirectX12 Harware error." << std::endl; }
	hr = D3D12CreateDevice(use_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
	if (FAILED(hr)) { throw std::runtime_error("D3D12CreateDevice is failed.");	}

	// �R�}���h�L���[����
	D3D12_COMMAND_QUEUE_DESC queue_desc{ D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0 };
	m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_command_queue));

	// HWND����N���C�A���g�̈�T�C�Y���擾
	RECT rect;
	GetClientRect(hWnd, &rect);
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;
	
	// �X���b�v�`�F�C������
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
	swap_chain_desc.BufferCount = m_frame_buffer_count;
	swap_chain_desc.Width = width;
	swap_chain_desc.Height = height;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.SampleDesc.Count = 1;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain;
	factory->CreateSwapChainForHwnd(m_command_queue.Get(), hWnd, &swap_chain_desc, nullptr, nullptr, &swap_chain);
	swap_chain.As(&m_swap_chain);

	// �f�B�X�N���v�^�q�[�v����
	PrepareDescriptorHeaps();
	// �����_�[�^�[�Q�b�g�r���[����
	PrepareRenderTargetView();
	// �f�v�X�o�b�t�@�֘A����
	CreateDepthBuffer(width, height);
}

void my_lib::D3D12AppBase::Terminate()
{

}
