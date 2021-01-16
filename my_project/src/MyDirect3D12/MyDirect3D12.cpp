#include "MyDirect3D12.hpp"

namespace mla
{
	MyDirect3D12::MyDirect3D12(const HWND &hwnd, const int &window_width, const int &window_height, const std::wstring &adapter_name):
		m_fence_value(0)
	{
		// �A�_�v�^�̎擾
#ifdef _DEBUG
		EnableDebugLayer(); // �f�o�b�O���C���̗L����
		CheckSuccess(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(m_dxgi_factory.ReleaseAndGetAddressOf())), "ERROR: CreateDXGIFactory1");
#else
		CheckSuccess(CreateDXGIFactory1(IID_PPV_ARGS(m_dxgi_factory.ReleaseAndGetAddressOf())), "ERROR: CreateDXGIFactory1");
#endif
		WRL::ComPtr<IDXGIAdapter> adapter = GetHardwareAdapter(adapter_name);
		// �f�o�C�X�I�u�W�F�N�g�̍쐬
		CheckSuccess(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf())), "ERROR: D3D12CreateDevice"); // MONZA
		
		// �R�}���h�A���P�[�^�̍쐬
		CheckSuccess(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmd_allocator.ReleaseAndGetAddressOf())), "ERROR: CreateCommandAllocator");
		// �R�}���h���X�g�̍쐬
		CheckSuccess(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd_allocator.Get(), nullptr, IID_PPV_ARGS(m_cmd_list.ReleaseAndGetAddressOf())), "ERROR: CreateCommandList");
		// �R�}���h�L���[�̍쐬
		D3D12_COMMAND_QUEUE_DESC cmd_queue_desc = {};
		cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // �S�R�}���h���X�g�����s�\
		cmd_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; // �f�t�H���g
		cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // �f�t�H���g
		cmd_queue_desc.NodeMask = 0; // 1��GPU�̂�
		CheckSuccess(m_device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(m_cmd_queue.ReleaseAndGetAddressOf())), "ERROR: CreateCommandQueue");

		// �X���b�v�`�F�[���̍쐬
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
		swap_chain_desc.Width = window_width;
		swap_chain_desc.Height = window_height;
		swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �s�N�Z���t�H�[�}�b�g
		swap_chain_desc.Stereo = false; // ���̎�
		swap_chain_desc.SampleDesc.Count = 1; // �T���v����(AA)
		swap_chain_desc.SampleDesc.Quality = 0; // �T���v���i��(AA)
		swap_chain_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // �g�p�@
		swap_chain_desc.BufferCount = 2;
		swap_chain_desc.Scaling = DXGI_SCALING_STRETCH; // �L�k
		swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // �t���[���o�b�t�@�̓��e�ێ�
		swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // �A���t�@�l��GPU�ɔC����
		swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // �E�B���h�E�ƃt���X�N�̐؂�ւ��\
		CheckSuccess(m_dxgi_factory->CreateSwapChainForHwnd(m_cmd_queue.Get(), hwnd, &swap_chain_desc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(m_dxgi_swap_chain.ReleaseAndGetAddressOf())), "ERROR: CreateSwapChainForHwnd");

		// �f�B�X�N���v�^�q�[�v�̍쐬(RTV)
		D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
		rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtv_heap_desc.NodeMask = 0;
		rtv_heap_desc.NumDescriptors = 2;
		rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CheckSuccess(m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(m_rtv_heaps.ReleaseAndGetAddressOf())), "ERROR: CreateDescriptorHeap");

		// �o�b�t�@�ƃf�B�X�N���v�^�̊֘A�t��
		m_back_buffers.resize(swap_chain_desc.BufferCount);
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtv_heaps->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < swap_chain_desc.BufferCount; ++i)
		{
			CheckSuccess(m_dxgi_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_back_buffers[i])), "ERROR: GetBuffer");
			m_device->CreateRenderTargetView(m_back_buffers[i], nullptr, handle); // D3D12: Removing Device.
			handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		// �t�F���X�̍쐬
		CheckSuccess(m_device->CreateFence(m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())), "ERROR: CreateFence");
	}

	MyDirect3D12::~MyDirect3D12()
	{
	}

	// �f�o�b�O���C���̗L����
	void MyDirect3D12::EnableDebugLayer()
	{
		ID3D12Debug* debug_layer = nullptr;
		CheckSuccess(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_layer)), "ERROR: D3D12GetDebugInterface");
		debug_layer->EnableDebugLayer();
		debug_layer->Release();
	}

	// ��ʃN���A
	void MyDirect3D12::ClearRenderTarget(const FLOAT *col)
	{
		const auto rtv_h = m_rtv_heaps->GetCPUDescriptorHandleForHeapStart();
		m_cmd_list->ClearRenderTargetView(rtv_h, col, 0, nullptr);
	}
	// �`��J�n����
	void MyDirect3D12::BeginDraw()
	{
		// �����_�[�^�[�Q�b�g�̐ݒ�
		auto bb_idx = m_dxgi_swap_chain->GetCurrentBackBufferIndex();
		auto rtv_h = m_rtv_heaps->GetCPUDescriptorHandleForHeapStart();
		const auto rtv_inc_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		rtv_h.ptr += (static_cast<SIZE_T>(bb_idx) * static_cast<SIZE_T>(rtv_inc_size));
		m_cmd_list->OMSetRenderTargets(1, &rtv_h, false, nullptr);
	}
	// �`��I������
	void MyDirect3D12::EndDraw()
	{
		// �N���[�Y�R�}���h
		m_cmd_list->Close();

		// �R�}���h���X�g�̎��s
		ID3D12CommandList* cmd_lists[] = {m_cmd_list.Get()};
		m_cmd_queue->ExecuteCommandLists(1, cmd_lists);

		// GPU�̏����҂�
		m_cmd_queue->Signal(m_fence.Get(), ++m_fence_value);
		if (m_fence->GetCompletedValue() != m_fence_value)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			m_fence->SetEventOnCompletion(m_fence_value, event);

			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		// ���Z�b�g
		m_cmd_allocator->Reset();
		m_cmd_list->Reset(m_cmd_allocator.Get(), nullptr);
	}
	
	// �R�}���h���X�g�̎擾
	WRL::ComPtr<ID3D12GraphicsCommandList> MyDirect3D12::GetCommandList()
	{
		return m_cmd_list;
	}
	// �X���b�v�`�F�[���̎擾
	WRL::ComPtr<IDXGISwapChain4> MyDirect3D12::GetSwapChain()
	{
		return m_dxgi_swap_chain;
	}

	// private 
	// �G���[�`�F�b�N����
	void MyDirect3D12::CheckSuccess(const HRESULT& is_ok, const std::string &err_msg)
	{
		if (FAILED(is_ok))
		{
			std::cerr << err_msg << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}
	WRL::ComPtr<IDXGIAdapter> MyDirect3D12::GetHardwareAdapter(const std::wstring& adapter_name)
	{
		// �A�_�v�^��
		std::vector<WRL::ComPtr<IDXGIAdapter>> adapters;
		for (int i = 0;; ++i)
		{
			WRL::ComPtr<IDXGIAdapter> tmp = nullptr;
			if (m_dxgi_factory->EnumAdapters(i, tmp.ReleaseAndGetAddressOf()) == DXGI_ERROR_NOT_FOUND) { break; }
			adapters.emplace_back(tmp);
		}
		// �A�_�v�^�擾
		WRL::ComPtr<IDXGIAdapter> adapter = nullptr;
		for (WRL::ComPtr<IDXGIAdapter> adpt : adapters)
		{
			DXGI_ADAPTER_DESC adpt_desc{};
			adpt->GetDesc(&adpt_desc);
			
			std::wstring adpt_str = adpt_desc.Description;
			if (!adapter && adpt_str.find(adapter_name) != std::wstring::npos) { adapter = adpt; }
		}
		return adapter;
	}
}
