#include "MyDirect3D12.hpp"

namespace mla
{
	MyDirect3D12::MyDirect3D12(const HWND& hwnd, const int& window_width, const int& window_height, const std::wstring& adapter_name) :
		m_fence_value(0)
	{
		// アダプタの取得
#ifdef _DEBUG
		EnableDebugLayer(); // デバッグレイヤの有効化
		CheckSuccess(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(m_dxgi_factory.ReleaseAndGetAddressOf())), "ERROR: CreateDXGIFactory1");
#else
		CheckSuccess(CreateDXGIFactory1(IID_PPV_ARGS(m_dxgi_factory.ReleaseAndGetAddressOf())), "ERROR: CreateDXGIFactory1");
#endif
		WRL::ComPtr<IDXGIAdapter> adapter = GetHardwareAdapter(adapter_name);
		// デバイスオブジェクトの作成
		CheckSuccess(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf())), "ERROR: D3D12CreateDevice"); // MONZA

		// コマンドアロケータの作成
		CheckSuccess(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmd_allocator.ReleaseAndGetAddressOf())), "ERROR: CreateCommandAllocator");
		// コマンドリストの作成
		CheckSuccess(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd_allocator.Get(), nullptr, IID_PPV_ARGS(m_cmd_list.ReleaseAndGetAddressOf())), "ERROR: CreateCommandList");
		// コマンドキューの作成
		D3D12_COMMAND_QUEUE_DESC cmd_queue_desc = {};
		cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 全コマンドリストを実行可能
		cmd_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; // デフォルト
		cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // デフォルト
		cmd_queue_desc.NodeMask = 0; // 1つのGPUのみ
		CheckSuccess(m_device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(m_cmd_queue.ReleaseAndGetAddressOf())), "ERROR: CreateCommandQueue");

		// スワップチェーンの作成
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
		swap_chain_desc.Width = window_width;
		swap_chain_desc.Height = window_height;
		swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ピクセルフォーマット
		swap_chain_desc.Stereo = false; // 立体視
		swap_chain_desc.SampleDesc.Count = 1; // サンプル回数(AA)
		swap_chain_desc.SampleDesc.Quality = 0; // サンプル品質(AA)
		swap_chain_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // 使用法
		swap_chain_desc.BufferCount = 2;
		swap_chain_desc.Scaling = DXGI_SCALING_STRETCH; // 伸縮
		swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // フレームバッファの内容維持
		swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // アルファ値はGPUに任せる
		swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ウィンドウとフルスクの切り替え可能
		CheckSuccess(m_dxgi_factory->CreateSwapChainForHwnd(m_cmd_queue.Get(), hwnd, &swap_chain_desc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(m_dxgi_swap_chain.ReleaseAndGetAddressOf())), "ERROR: CreateSwapChainForHwnd");

		// ディスクリプタヒープの作成(RTV)
		D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
		rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtv_heap_desc.NodeMask = 0;
		rtv_heap_desc.NumDescriptors = 2;
		rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CheckSuccess(m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(m_rtv_heaps.ReleaseAndGetAddressOf())), "ERROR: CreateDescriptorHeap");

		// バッファとディスクリプタの関連付け
		m_back_buffers.resize(swap_chain_desc.BufferCount);
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtv_heaps->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < swap_chain_desc.BufferCount; ++i)
		{
			CheckSuccess(m_dxgi_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_back_buffers[i])), "ERROR: GetBuffer");
			m_device->CreateRenderTargetView(m_back_buffers[i].Get(), nullptr, handle); // D3D12: Removing Device.
			handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}
		// フェンスの作成
		CheckSuccess(m_device->CreateFence(m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())), "ERROR: CreateFence");

		// ビューポート
		m_view_port.Width = static_cast<FLOAT>(window_width);
		m_view_port.Height = static_cast<FLOAT>(window_height);
		m_view_port.TopLeftX = 0;
		m_view_port.TopLeftY = 0;
		m_view_port.MaxDepth = 1.0f;
		m_view_port.MinDepth = 0.0f;
		// シザー矩形
		m_scissor_rect.top = 0;
		m_scissor_rect.left = 0;
		m_scissor_rect.right = m_scissor_rect.left + static_cast<LONG>(window_width);
		m_scissor_rect.bottom = m_scissor_rect.top + static_cast<LONG>(window_height);
	}

	MyDirect3D12::~MyDirect3D12()
	{
#ifdef _DEBUG
		//EnableDebugReportObject(); // デバッグレポートの有効化
#endif
	}

	// デバッグレイヤの有効化
	void MyDirect3D12::EnableDebugLayer()
	{
		ID3D12Debug* debug_layer = nullptr;
		CheckSuccess(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_layer)), "ERROR: D3D12GetDebugInterface");
		debug_layer->EnableDebugLayer();
		debug_layer->Release();
	}
	// デバッグレポートの有効化
	void MyDirect3D12::EnableDebugReportObject()
	{
		ID3D12DebugDevice* debugInterface;
		if (SUCCEEDED(m_device.Get()->QueryInterface(&debugInterface)))
		{
			debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
			debugInterface->Release();
		}
	}

	// 画面クリア
	void MyDirect3D12::ClearRenderTarget(const FLOAT* col)
	{
		// バックバッファのインデックスを取得
		auto bb_idx = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

		// rtvの位置計算
		auto rtv_h = m_rtv_heaps->GetCPUDescriptorHandleForHeapStart();
		auto rtv_inc_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		rtv_h.ptr += (static_cast<SIZE_T>(bb_idx) * static_cast<SIZE_T>(rtv_inc_size));

		m_cmd_list->ClearRenderTargetView(rtv_h, col, 0, nullptr);
	}
	// 描画開始処理
	void MyDirect3D12::BeginDraw()
	{
		// バックバッファのインデックスを取得
		auto bb_idx = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

		// リソースバリアの設定
		D3D12_RESOURCE_BARRIER barrier_desc = {};
		barrier_desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_desc.Transition.pResource = m_back_buffers[bb_idx].Get();
		barrier_desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier_desc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier_desc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		m_cmd_list->ResourceBarrier(1, &barrier_desc);

		// レンダーターゲットの設定
		auto rtv_h = m_rtv_heaps->GetCPUDescriptorHandleForHeapStart();
		const auto rtv_inc_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		rtv_h.ptr += (static_cast<SIZE_T>(bb_idx) * static_cast<SIZE_T>(rtv_inc_size));
		m_cmd_list->OMSetRenderTargets(1, &rtv_h, false, nullptr);
	}
	// 描画終了処理
	void MyDirect3D12::EndDraw()
	{
		// バックバッファのインデックスを取得
		auto bb_idx = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

		// リソースバリアの設定
		D3D12_RESOURCE_BARRIER barrier_desc = {};
		barrier_desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_desc.Transition.pResource = m_back_buffers[bb_idx].Get();
		barrier_desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier_desc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier_desc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		m_cmd_list->ResourceBarrier(1, &barrier_desc);

		// クローズコマンド
		m_cmd_list->Close();

		// コマンドリストの実行
		ID3D12CommandList* cmd_lists[] = { m_cmd_list.Get() };
		m_cmd_queue->ExecuteCommandLists(1, cmd_lists);

		// GPUの処理待ち
		m_cmd_queue->Signal(m_fence.Get(), ++m_fence_value);
		if (m_fence->GetCompletedValue() != m_fence_value)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			m_fence->SetEventOnCompletion(m_fence_value, event);

			if (event)
			{
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}
		}

		// リセット
		m_cmd_allocator->Reset();
		m_cmd_list->Reset(m_cmd_allocator.Get(), nullptr);

		m_dxgi_swap_chain->Present(1, 0);
	}

	// Blobエラー処理
	void MyDirect3D12::ErrorBlob(WRL::ComPtr<ID3DBlob>& err_blob)
	{
		std::string msg;
		msg.resize(err_blob->GetBufferSize());
		std::copy_n(reinterpret_cast<char*>(err_blob->GetBufferPointer()), err_blob->GetBufferSize(), msg.begin());
		std::cout << msg << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// リソース作成
	WRL::ComPtr<ID3D12Resource> MyDirect3D12::CreateCommitedResource(const D3D12_HEAP_PROPERTIES& heap_prop, const D3D12_RESOURCE_DESC& desc)
	{
		WRL::ComPtr<ID3D12Resource> buff = nullptr;
		CheckSuccess(m_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buff)), "ERROR: CreateCommitedResource");
		return buff;
	}

	// 簡易版シェーダ作成
	void MyDirect3D12::CompileBasicShader(const std::wstring& vs_path, const std::wstring& ps_path, D3D12_GRAPHICS_PIPELINE_STATE_DESC* g_pipeline)
	{
		WRL::ComPtr<ID3DBlob> err_blob = nullptr;
		// 頂点シェーダ
		WRL::ComPtr<ID3DBlob> vs_blob = nullptr;
		HRESULT vs_result = D3DCompileFromFile(vs_path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "basicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vs_blob.ReleaseAndGetAddressOf(), err_blob.ReleaseAndGetAddressOf());
		if (vs_result != S_OK) { ErrorBlob(err_blob); }
		// ピクセルシェーダ
		WRL::ComPtr<ID3DBlob> ps_blob = nullptr;
		HRESULT ps_result = D3DCompileFromFile(ps_path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "basicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, ps_blob.ReleaseAndGetAddressOf(), err_blob.ReleaseAndGetAddressOf());
		if (ps_result != S_OK) { ErrorBlob(err_blob); }

		// 頂点レイアウト
		D3D12_INPUT_ELEMENT_DESC input_layout[] =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
			},
		};
		// パイプラインステートにシェーダを設定
		g_pipeline->VS.pShaderBytecode = vs_blob->GetBufferPointer();
		g_pipeline->VS.BytecodeLength = vs_blob->GetBufferSize();
		g_pipeline->PS.pShaderBytecode = ps_blob->GetBufferPointer();
		g_pipeline->PS.BytecodeLength = ps_blob->GetBufferSize();
		// サンプルマスク，ラスタライザステート設定
		g_pipeline->SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		g_pipeline->RasterizerState.MultisampleEnable = false;
		g_pipeline->RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		g_pipeline->RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		g_pipeline->RasterizerState.DepthClipEnable = true;
		// ブレンドステート設定
		D3D12_RENDER_TARGET_BLEND_DESC rt_blend_desc = {};
		rt_blend_desc.BlendEnable = false;
		rt_blend_desc.LogicOpEnable = false;
		rt_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		g_pipeline->BlendState.AlphaToCoverageEnable = false;
		g_pipeline->BlendState.IndependentBlendEnable = false;
		g_pipeline->BlendState.RenderTarget[0] = rt_blend_desc;
		// 入力レイアウト設定
		g_pipeline->InputLayout.pInputElementDescs = input_layout;
		g_pipeline->InputLayout.NumElements = _countof(input_layout);
		g_pipeline->IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		g_pipeline->PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// レンダターゲット設定
		g_pipeline->NumRenderTargets = 1;
		g_pipeline->RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		// AA設定
		g_pipeline->SampleDesc.Count = 1;
		g_pipeline->SampleDesc.Quality = 0;

		// パイプラインステート生成
		CheckSuccess(m_device->CreateGraphicsPipelineState(g_pipeline, IID_PPV_ARGS(m_pipeline_state.ReleaseAndGetAddressOf())), "ERROR: CreateGraphicsPipelineState");
	}

	// パイプラインステート設定
	void MyDirect3D12::SetPipelineState()
	{
		m_cmd_list->SetPipelineState(m_pipeline_state.Get());
	}
	// ルートシグネチャ設定
	void MyDirect3D12::SetGraphicsRootSignature(const WRL::ComPtr<ID3D12RootSignature> &root_signature)
	{
		m_cmd_list->SetGraphicsRootSignature(root_signature.Get());
	}
	// ビュー，シザー矩形設定
	void MyDirect3D12::SetViewAndScissor()
	{
		m_cmd_list->RSSetViewports(1, &m_view_port);
		m_cmd_list->RSSetScissorRects(1, &m_scissor_rect);
	}
	// トポロジ設定
	void MyDirect3D12::SetPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY &type)
	{
		m_cmd_list->IASetPrimitiveTopology(type);
	}

	// デバイス取得
	WRL::ComPtr<ID3D12Device> MyDirect3D12::GetDevice()
	{
		return m_device;
	}
	// コマンドリストの取得
	WRL::ComPtr<ID3D12GraphicsCommandList> MyDirect3D12::GetCommandList()
	{
		return m_cmd_list;
	}
	// スワップチェーンの取得
	WRL::ComPtr<IDXGISwapChain4> MyDirect3D12::GetSwapChain()
	{
		return m_dxgi_swap_chain;
	}

	// private 
	// エラーチェック処理
	void MyDirect3D12::CheckSuccess(const HRESULT& is_ok, const std::string& err_msg)
	{
		if (FAILED(is_ok))
		{
			std::cerr << err_msg << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}
	WRL::ComPtr<IDXGIAdapter> MyDirect3D12::GetHardwareAdapter(const std::wstring& adapter_name)
	{
		// アダプタ列挙
		std::vector<WRL::ComPtr<IDXGIAdapter>> adapters;
		for (int i = 0;; ++i)
		{
			WRL::ComPtr<IDXGIAdapter> tmp = nullptr;
			if (m_dxgi_factory->EnumAdapters(i, tmp.ReleaseAndGetAddressOf()) == DXGI_ERROR_NOT_FOUND) { break; }
			adapters.emplace_back(tmp);
		}
		// アダプタ取得
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
