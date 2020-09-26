#include <Windows.h>

#include <DirectXTex.h>

#include <d3dx12.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

#include <iostream>
#include <vector>
#include <string>

#define OUT() std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);

// �O���[�o���ϐ�
const int window_width = 1000;
const int window_height = 680;
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

// �|���S�����
struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};

// �e�N�X�`�����
const DirectX::Image* img = nullptr;
DirectX::TexMetadata tex_test_meta{};
DirectX::ScratchImage tex_test_img{};

// PMD
struct PMDHeader
{
	float version;       // �o�[�W����
	char model_name[20]; // ���f����
	char comment[256];   // ���f���R�����g
};

PMDHeader pmd_header;

struct PMDVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	
	unsigned short bone_no[2];
	unsigned char bone_weight;
	unsigned char edge_flg;
};

constexpr size_t pmd_vertex_size = 38;

// �V�F�[�_�֓n���s��f�[�^
struct MatricesData
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view_projection;
};

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
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
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
	if (FAILED(p_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&rtv_heaps)))) { OUT(); }

	// sRGB�����_�^�[�Q�b�g�r���[�ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
	rtv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	// �X���b�v�`�F�[���ƃ������̕R�Â�
	DXGI_SWAP_CHAIN_DESC swc_desc = {};
	if (FAILED(p_swap_chain->GetDesc(&swc_desc))) { OUT(); }
	back_buffers.resize(swc_desc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_heaps->GetCPUDescriptorHandleForHeapStart();
	for (int index = 0; index < swc_desc.BufferCount; ++index)
	{
		if (FAILED(p_swap_chain->GetBuffer(index, IID_PPV_ARGS(&back_buffers[index])))) { OUT(); }
		p_device->CreateRenderTargetView(back_buffers[index], &rtv_desc, handle);
		handle.ptr += p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// �t�F���X����
	if (FAILED(p_device->CreateFence(fence_val, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&p_fence)))) { OUT(); }
}

size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
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

	// COM������
	if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))
	{
		OUT();
	}

	// �e�N�X�`���ǂݍ���
	if (FAILED(DirectX::LoadFromWICFile(L"../Resource/tex_test.png", DirectX::WIC_FLAGS_NONE, &tex_test_meta, tex_test_img)))
	{
		OUT();
	}
	// �摜���o
	img = tex_test_img.GetImage(0, 0, 0);

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
	
	// PMD�ǂݍ���
	char signature[3] = {};
	std::FILE* fp = nullptr;
	fopen_s(&fp, "../Model/�����~�N.pmd", "rb");
	// �w�b�_
	std::fread(signature, sizeof(signature), 1, fp);
	std::fread(&pmd_header, sizeof(pmd_header), 1, fp);
	// ���_
	unsigned int vertex_num = 0;
	std::fread(&vertex_num, sizeof(vertex_num), 1, fp);
	std::vector<unsigned char> vertices(vertex_num * pmd_vertex_size);
	std::fread(vertices.data(), vertices.size(), 1, fp);
	// �C���f�b�N�X
	unsigned int indices_num;
	std::fread(&indices_num, sizeof(indices_num), 1, fp);
	std::vector<unsigned short> indices(indices_num);
	std::fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	std::fclose(fp);

	// ���[���h�s��
	float angle = 0.0f;
	DirectX::XMMATRIX world_matrix = DirectX::XMMatrixIdentity();
	// �r���[�s��
	DirectX::XMFLOAT3 eye(0.0f, 10.0f, -15.0f);
	DirectX::XMFLOAT3 target(0.0f, 10.0f, 0.0f);
	DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
	// �v���W�F�N�V�����s��
	DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveLH(DirectX::XM_PIDIV2, static_cast<float>(window_width) / static_cast<float>(window_height), 1.0f, 100.0f);

	// �萔�o�b�t�@�쐬
	ID3D12Resource* const_buffer = nullptr;
	if (FAILED(p_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer((sizeof(MatricesData) + 0xff) & ~0xff), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&const_buffer))))
	{
		OUT();
	}
	// �萔�R�s�[
	MatricesData* map_matrix = nullptr;
	if (FAILED(const_buffer->Map(0, nullptr, (void**)&map_matrix))) { OUT(); }
	map_matrix->world = world_matrix;
	map_matrix->view_projection = view * projection;

	// ���_�o�b�t�@����
	D3D12_HEAP_PROPERTIES heap_prop = {};
	heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	// ���\�[�X�ݒ�
	ID3D12Resource* vert_buff = nullptr;
	if (FAILED(p_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertices.size()), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vert_buff))))
	{
		OUT();
	}

	// ���_�f�[�^�̃}�b�v
	unsigned char* vert_map = nullptr;
	if (FAILED(vert_buff->Map(0, nullptr, (void**)&vert_map))) { OUT(); }
	std::copy(std::begin(vertices), std::end(vertices), vert_map);
	vert_buff->Unmap(0, nullptr);
	// ���_�o�b�t�@�r���[�쐬
	D3D12_VERTEX_BUFFER_VIEW vb_view = {};
	vb_view.BufferLocation = vert_buff->GetGPUVirtualAddress();
	vb_view.SizeInBytes = vertices.size();
	vb_view.StrideInBytes = pmd_vertex_size;
	// �C���f�b�N�X�o�b�t�@����
	ID3D12Resource* idx_buffer = nullptr;
	if (FAILED(p_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0])), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idx_buffer))))
	{
		OUT();
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
	ib_view.SizeInBytes = indices.size() * sizeof(indices[0]);
	
	// �A�b�v���[�h�p���\�[�X�쐬
	D3D12_HEAP_PROPERTIES upload_heap_prop = {};
	upload_heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	upload_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	upload_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	upload_heap_prop.CreationNodeMask = 0;
	upload_heap_prop.VisibleNodeMask = 0;
	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC res_desc = {};
	res_desc = {};
	res_desc.Format = DXGI_FORMAT_UNKNOWN;
	res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	res_desc.Width = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height;
	res_desc.Height = 1;
	res_desc.DepthOrArraySize = 1;
	res_desc.MipLevels = 1;
	res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	res_desc.SampleDesc.Count = 1;
	res_desc.SampleDesc.Quality = 0;
	// ���ԃo�b�t�@�쐬
	ID3D12Resource* upload_buff = nullptr;
	if (FAILED(p_device->CreateCommittedResource(&upload_heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_buff))))
	{
		OUT();
	}

	// �R�s�[�惊�\�[�X�쐬
	D3D12_HEAP_PROPERTIES tex_heap_prop = {};
	tex_heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
	tex_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	tex_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	tex_heap_prop.CreationNodeMask = 0;
	tex_heap_prop.VisibleNodeMask = 0;

	res_desc = {};
	res_desc.Format = tex_test_meta.format;
	res_desc.Width = tex_test_meta.width;
	res_desc.Height = tex_test_meta.height;
	res_desc.DepthOrArraySize = tex_test_meta.arraySize;
	res_desc.SampleDesc.Count = tex_test_meta.mipLevels;
	res_desc.SampleDesc.Quality = 0;
	res_desc.MipLevels = 1;
	res_desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(tex_test_meta.dimension);
	res_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// �e�N�X�`���o�b�t�@�̍쐬
	ID3D12Resource* tex_buff = nullptr;
	if (FAILED(p_device->CreateCommittedResource(&tex_heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&tex_buff))))
	{
		OUT();
	}

	// �A�b�v���[�h���\�[�X�ւ̃}�b�v
	uint8_t* map_for_img = nullptr;
	if (FAILED(upload_buff->Map(0, nullptr, (void**)&map_for_img))) { OUT(); }

	auto src_address = img->pixels;
	auto row_pitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	for (int y = 0; y < img->height; ++y)
	{
		std::copy_n(src_address, row_pitch, map_for_img);
		src_address += img->rowPitch;
		map_for_img += row_pitch;
	}
	upload_buff->Unmap(0, nullptr);

	// ���\�[�X�փR�s�[
	UINT nrow;
	UINT64 rowsize, size;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	auto desc = tex_buff->GetDesc();
	p_device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);

	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.PlacedFootprint = footprint;
	src.pResource = upload_buff;
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = tex_test_meta.width;
	src.PlacedFootprint.Footprint.Height = tex_test_meta.height;
	src.PlacedFootprint.Footprint.Depth = tex_test_meta.depth;
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = tex_buff;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	p_cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	// �o���A�ƃt�F���X�̐ݒ�
	UINT bb_idx = p_swap_chain->GetCurrentBackBufferIndex();
	p_cmd_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex_buff, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
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
	p_cmd_allocator->Reset();
	p_cmd_list->Reset(p_cmd_allocator, nullptr);

	// �f�B�X�N���v�^�q�[�v�쐬
	ID3D12DescriptorHeap* basic_desc_heap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc = {};
	desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc_heap_desc.NodeMask = 0;
	desc_heap_desc.NumDescriptors = 2; // SRV, CBV
	desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (FAILED(p_device->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&basic_desc_heap)))) { OUT(); }

	// �r���[�ݒ�
	auto basic_heap_handle = basic_desc_heap->GetCPUDescriptorHandleForHeapStart();
	// �V�F�[�_���\�[�X�r���[�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = tex_test_meta.format;
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;
	p_device->CreateShaderResourceView(tex_buff, &srv_desc, basic_heap_handle);
	// �萔�o�b�t�@�r���[�쐬
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
	cbv_desc.BufferLocation = const_buffer->GetGPUVirtualAddress();
	cbv_desc.SizeInBytes = const_buffer->GetDesc().Width;
	basic_heap_handle.ptr += p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	p_device->CreateConstantBufferView(&cbv_desc, basic_heap_handle);

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
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"BONE_NO", 0, DXGI_FORMAT_R16G16_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		/*
		{
			"EDGE_FLG", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		*/
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
	g_pipeline_desc.DepthStencilState.DepthEnable = true;
	g_pipeline_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	g_pipeline_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	g_pipeline_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
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
	g_pipeline_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// �A���`�G�C���A�V���O�ݒ�
	g_pipeline_desc.SampleDesc.Count = 1;
	g_pipeline_desc.SampleDesc.Quality = 0;

	// �f�B�X�N���v�^�����W����
	D3D12_DESCRIPTOR_RANGE desc_tbl_range[2] = {};
	// �e�N�X�`���p: 0��
	desc_tbl_range[0].NumDescriptors = 1;
	desc_tbl_range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	desc_tbl_range[0].BaseShaderRegister = 0;
	desc_tbl_range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	// �萔�p: 1��
	desc_tbl_range[1].NumDescriptors = 1;
	desc_tbl_range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	desc_tbl_range[1].BaseShaderRegister = 0;
	desc_tbl_range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ���[�g�p�����[�^�̍쐬
	D3D12_ROOT_PARAMETER root_param = {};
	root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	root_param.DescriptorTable.pDescriptorRanges = &desc_tbl_range[0];
	root_param.DescriptorTable.NumDescriptorRanges = 2;
	root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// �T���v���ݒ�
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

	// �[�x�o�b�t�@
	D3D12_RESOURCE_DESC depth_res_desc = {};
	depth_res_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth_res_desc.Width = window_width;
	depth_res_desc.Height = window_height;
	depth_res_desc.DepthOrArraySize = 1;
	depth_res_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_res_desc.SampleDesc.Count = 1;
	depth_res_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	// �[�x�l�p�q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES depth_heap_prop = {};
	depth_heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
	depth_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depth_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	// �N���A�o�����[
	D3D12_CLEAR_VALUE depth_clear_value = {};
	depth_clear_value.DepthStencil.Depth = 1.0f;
	depth_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
	
	ID3D12Resource* depth_buffer = nullptr;
	if (FAILED(p_device->CreateCommittedResource(&depth_heap_prop, D3D12_HEAP_FLAG_NONE, &depth_res_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depth_clear_value, IID_PPV_ARGS(&depth_buffer))))
	{
		OUT();
	}

	// �[�x�p�f�B�X�N���v�^�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
	dsv_heap_desc.NumDescriptors = 1;
	dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	ID3D12DescriptorHeap* dsv_heap = nullptr;
	if (FAILED(p_device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&dsv_heap)))) { OUT(); }

	// �f�v�X�X�e���V���r���[
	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
	
	p_device->CreateDepthStencilView(depth_buffer, &dsv_desc, dsv_heap->GetCPUDescriptorHandleForHeapStart());

	// ���[�g�V�O�l�`���̍쐬
	ID3D12RootSignature* p_root_signature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	root_signature_desc.pParameters = &root_param;
	root_signature_desc.NumParameters = 1;
	root_signature_desc.pStaticSamplers = &sampler_desc;
	root_signature_desc.NumStaticSamplers = 1;

	ID3DBlob* root_signature_blob = nullptr;
	if (FAILED(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &root_signature_blob, &error_blob)))
	{
		OUT();
	}

	if (FAILED(p_device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&p_root_signature))))
	{
		OUT();
	}
	root_signature_blob->Release();

	g_pipeline_desc.pRootSignature = p_root_signature;

	// �O���t�B�b�N�p�C�v���C���X�e�[�g�I�u�W�F�N�g����
	if (FAILED(p_device->CreateGraphicsPipelineState(&g_pipeline_desc, IID_PPV_ARGS(&p_pipeline_state)))) { OUT(); }

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

		// �s��v�Z
		world_matrix = DirectX::XMMatrixRotationY(angle);
		map_matrix->world = world_matrix;
		map_matrix->view_projection = view* projection;
		angle += 1.0f / (32.0f);
		
		// D3D12
		UINT bb_idx = p_swap_chain->GetCurrentBackBufferIndex();
		// �o���A�ݒ�
		p_cmd_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(back_buffers[bb_idx], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		
		p_cmd_list->SetPipelineState(p_pipeline_state);

		// RT�ݒ�
		auto rtv_h = rtv_heaps->GetCPUDescriptorHandleForHeapStart();
		rtv_h.ptr += bb_idx * p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		auto dsv_h = dsv_heap->GetCPUDescriptorHandleForHeapStart();
		p_cmd_list->OMSetRenderTargets(1, &rtv_h, false, &dsv_h);

		// RT�N���A
		float clear_color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		p_cmd_list->ClearRenderTargetView(rtv_h, clear_color, 0, nullptr);
		// �[�x�o�b�t�@�N���A
		p_cmd_list->ClearDepthStencilView(dsv_h, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// �`�施��
		p_cmd_list->RSSetViewports(1, &view_port);
		p_cmd_list->RSSetScissorRects(1, &scissor_rect);
		p_cmd_list->SetGraphicsRootSignature(p_root_signature);
		p_cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		p_cmd_list->IASetVertexBuffers(0, 1, &vb_view);
		p_cmd_list->IASetIndexBuffer(&ib_view);

		p_cmd_list->SetGraphicsRootSignature(p_root_signature);
		p_cmd_list->SetDescriptorHeaps(1, &basic_desc_heap);
		p_cmd_list->SetGraphicsRootDescriptorTable(0, basic_desc_heap->GetGPUDescriptorHandleForHeapStart());

		p_cmd_list->DrawIndexedInstanced(indices_num, 1, 0, 0, 0);

		p_cmd_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(back_buffers[bb_idx], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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