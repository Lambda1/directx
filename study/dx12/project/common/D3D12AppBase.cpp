#include "D3D12AppBase.hpp"

my_lib::D3D12AppBase::D3D12AppBase()
{
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

// public
void my_lib::D3D12AppBase::Initialize(HWND hWnd)
{
	HRESULT hr = FALSE;
	UINT dxgi_flags = 0;

	// DebugLayer
#if (_DEBUG)
	DebugMode(&dxgi_flags);
#endif

	// DXGI Factory����
	Microsoft::WRL::ComPtr<IDXGIFactory3> factory;
	hr = CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(&factory));
	if (FAILED(hr)) { std::runtime_error("CreateDXGIFactory2 is failed."); }

	// D3D12�f�o�C�X����
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter = SearchHardwareAdapter(factory);
	Microsoft::WRL::ComPtr<IDXGIAdapter1> use_adapter;
	if (adapter.Get())
	{
		// �n�[�h�E�F�A�A�_�v�^�ݒ�
		adapter.As(&use_adapter);
	}
	else
	{
		// WARP�ݒ�
		// TODO: �n�[�h�E�F�A�A�_�v�^���Ȃ��ꍇ, WARP��ݒ�
		std::cout << "DirectX12 Harware error." << std::endl;
	}
	hr = D3D12CreateDevice(use_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
	if (FAILED(hr)) { std::runtime_error("D3D12CreateDevice is failed.");	}
}

void my_lib::D3D12AppBase::Terminate()
{

}
