#include "./MyD3D9.hpp"

MyD3D9::MyD3D9(const std::string& window_title, const int& width, const int& height) :
	m_window_width(width), m_window_height(height), m_window_title_name(window_title),
	m_hwnd{}, m_window_class{},
	m_p_direct3d(nullptr), m_p_direct3d_device(nullptr)
{
	// WindowClass�o�^
	m_window_class =
	{
		sizeof(WNDCLASSEX), CS_CLASSDC, MyD3D9::MessageProc,
		0L, 0L, GetModuleHandle(nullptr),
		nullptr, nullptr, nullptr,
		nullptr, m_window_class_name.c_str(), nullptr
	};
	RegisterClassEx(&m_window_class);

	// WindowApplication�쐬
	m_hwnd = CreateWindow
	(
		m_window_class_name.c_str(), m_window_title_name.c_str(), WS_OVERLAPPEDWINDOW,
		m_init_window_x, m_init_window_y, m_window_width, m_window_height,
		GetDesktopWindow(), nullptr, m_window_class.hInstance, nullptr
	);

	// Direct3D������
	InitDirect3D();

	// Show Window
	ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	UpdateWindow(m_hwnd);
}

MyD3D9::~MyD3D9()
{
	// Direct3D: �������
	CleanUp();

	// WindowAPI: �������
	bool is_check = UnregisterClass(m_window_class_name.c_str(), m_window_class.hInstance);
	if (!is_check)
	{
		std::cerr << "ERROR " << __LINE__ << ": UnregisterClass returns 0." << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

// private

// �R�[���o�b�N: �E�B���h�E���b�Z�[�W
LRESULT WINAPI MyD3D9::MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		ValidateRect(hWnd, nullptr);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Direct3D�̏�����
void MyD3D9::InitDirect3D()
{
	// �R���|�[�l���g�擾
	m_p_direct3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_p_direct3d)
	{
		std::cerr << "ERROR: Direct3DCreate9 is NULL." << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// �C���^�[�t�F�[�X��`
	D3DPRESENT_PARAMETERS d3d_pp;
	// �E�B���h�E���[�h
	d3d_pp.Windowed = TRUE;
	// �o�b�N�o�b�t�@�T�C�Y
	d3d_pp.BackBufferWidth = m_window_width;
	d3d_pp.BackBufferHeight = m_window_height;
	// �o�b�N�o�b�t�@�F��
	d3d_pp.BackBufferFormat = D3DFMT_UNKNOWN;
	// �o�b�N�o�b�t�@��
	d3d_pp.BackBufferCount = 1;
	// �}���`�T���v�����O
	d3d_pp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3d_pp.MultiSampleQuality = 0;
	// �X���b�s���O
	d3d_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// �E�B���h�E�n���h���w��
	d3d_pp.hDeviceWindow = nullptr;
	// �[�x�X�e���V���o�b�t�@
	d3d_pp.EnableAutoDepthStencil = TRUE;
	// �X�e���V���o�b�t�@�t�H�[�}�b�g
	d3d_pp.AutoDepthStencilFormat = D3DFMT_D16;
	// �o�b�N�o�b�t�@����t�����g�o�b�t�@�ւ̓]���I�v�V����
	d3d_pp.Flags = 0;
	// ���t���b�V�����[�g
	d3d_pp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	// ���������^�C�~���O
	d3d_pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// �f�o�C�X�擾
	HRESULT is_device_create = m_p_direct3d->CreateDevice
	(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3d_pp, &m_p_direct3d_device
	);
	if (FAILED(is_device_create))
	{
		std::cerr << "ERROR " << __LINE__ << ": CreateDevice is failed." << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

// Direct3D �I������
void MyD3D9::CleanUp()
{
	if (m_p_direct3d_device) { m_p_direct3d_device->Release(); }
	if (m_p_direct3d) { m_p_direct3d->Release(); }
}

// public

// ���C������
void MyD3D9::MainLoop()
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
