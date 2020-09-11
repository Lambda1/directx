#include <Windows.h>

#include <d3d11.h>
#include <D3DX10.h>
#include <d3dcompiler.h>
#include <xnamath.h>

/* グローバル変数 */
// ウィンドウ設定
HWND g_hWnd = nullptr;
LPCWSTR	w_name = L"Window";
const int w_width = 640, w_height = 480;

/* DirectX11 */
// D3D11
ID3D11Device* g_p_device;
ID3D11DeviceContext* g_p_device_context;
IDXGISwapChain* g_p_swap_chain;
ID3D11RenderTargetView* g_p_rtv;
ID3D11Texture2D* g_p_ds;
ID3D11DepthStencilView* g_p_dsv;
// Shader
ID3D11InputLayout* g_p_vertex_layout;
ID3D11VertexShader* g_p_vertex_shader;
ID3D11PixelShader* g_p_pixel_shader;
ID3D11Buffer* g_p_constant_buffer, * g_p_vertex_buffer;

// 頂点用の構造体
struct SimpleVertex
{
	D3DXVECTOR3 m_pos;
};
// Simpleシェーダ用のコンスタントバッファのアプリ側構造体
struct SIMPLESHADER_CONSTANT_BUFFER
{
	D3DXMATRIX m_wvp;
};

/* DX11関係 */
// シェーダ作成
HRESULT MakeShader(const char* sz_file_name, const char* sz_func_name, const char* sz_profile_name, void** pp_shader, ID3DBlob** pp_blob)
{
	ID3DBlob* p_errors = nullptr;

	if (FAILED(D3DX10CompileFromFileA(sz_file_name, nullptr, nullptr, sz_func_name, sz_profile_name, D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION, 0, nullptr, pp_blob, &p_errors, nullptr)))
	{
		char* p = reinterpret_cast<char*>(p_errors->GetBufferPointer());
		MessageBoxA(0, p, 0, MB_OK);
		return E_FAIL;
	}

	char sz_profile[3] = {};
	memcpy(sz_profile, sz_profile_name, 2);
	if(strcmp(sz_profile,"vs")==0) { g_p_device->CreateVertexShader((*pp_blob)->GetBufferPointer(),(*pp_blob)->GetBufferSize(),NULL,reinterpret_cast<ID3D11VertexShader**>(pp_shader)); }
	if(strcmp(sz_profile,"ps")==0) { g_p_device->CreatePixelShader((*pp_blob)->GetBufferPointer(),(*pp_blob)->GetBufferSize(),NULL,reinterpret_cast<ID3D11PixelShader**>(pp_shader)); }
	if(strcmp(sz_profile,"gs")==0) { g_p_device->CreateGeometryShader((*pp_blob)->GetBufferPointer(),(*pp_blob)->GetBufferSize(),NULL,reinterpret_cast<ID3D11GeometryShader**>(pp_shader)); }
	if(strcmp(sz_profile,"hs")==0) { g_p_device->CreateHullShader((*pp_blob)->GetBufferPointer(),(*pp_blob)->GetBufferSize(),NULL,reinterpret_cast<ID3D11HullShader**>(pp_shader)); }
	if(strcmp(sz_profile,"ds")==0) { g_p_device->CreateDomainShader((*pp_blob)->GetBufferPointer(),(*pp_blob)->GetBufferSize(),NULL,reinterpret_cast<ID3D11DomainShader**>(pp_shader)); }
	if(strcmp(sz_profile,"cs")==0) { g_p_device->CreateComputeShader((*pp_blob)->GetBufferPointer(),(*pp_blob)->GetBufferSize(),NULL,reinterpret_cast<ID3D11ComputeShader**>(pp_shader)); }

	return S_OK;
}

// D3D11: 初期化
HRESULT InitDirect3D()
{
	// デバイス・スワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd;

	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = w_width;
	sd.BufferDesc.Height = w_height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL feature_levels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* p_feature_level = nullptr;

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &feature_levels, 1, D3D11_SDK_VERSION, &sd, &g_p_swap_chain, &g_p_device, p_feature_level, &g_p_device_context);

	// バックバッファのレンダーターゲットビュー(RTV)を作成
	ID3D11Texture2D* p_back;
	
	g_p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&p_back);
	g_p_device->CreateRenderTargetView(p_back, nullptr, &g_p_rtv);
	p_back->Release();

	// デプスステンシルビュー(DSV)を作成
	D3D11_TEXTURE2D_DESC desc_depth;

	desc_depth.Width = w_width;
	desc_depth.Height = w_height;
	desc_depth.MipLevels = 1;
	desc_depth.ArraySize = 1;
	desc_depth.Format = DXGI_FORMAT_D32_FLOAT;
	desc_depth.SampleDesc.Count = 1;
	desc_depth.SampleDesc.Quality = 0;
	desc_depth.Usage = D3D11_USAGE_DEFAULT;
	desc_depth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc_depth.CPUAccessFlags = 0;
	desc_depth.MipLevels = 0;

	g_p_device->CreateTexture2D(&desc_depth, nullptr, &g_p_ds);
	g_p_device->CreateDepthStencilView(g_p_ds, nullptr, &g_p_dsv);

	// ビューポート設定
	D3D11_VIEWPORT vp;
	vp.Width = w_width;
	vp.Height = w_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	
	g_p_device_context->RSSetViewports(1, &vp);

	// VertexShader作成
	ID3DBlob* p_compiled_shader = nullptr;
	MakeShader("shader.hlsl", "VertexS", "vs_5_0", reinterpret_cast<void**>(&g_p_vertex_shader), &p_compiled_shader);
	// VertexInputLayoutを定義
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	UINT num_elements = sizeof(layout) / sizeof(layout[0]);
	// VertexInputLayoutを作成
	g_p_device->CreateInputLayout(layout, num_elements, p_compiled_shader->GetBufferPointer(), p_compiled_shader->GetBufferSize(), &g_p_vertex_layout);

	// PixelShader作成
	MakeShader("shader.hlsl", "PixelS", "ps_5_0", reinterpret_cast<void**>(&g_p_pixel_shader), &p_compiled_shader);

	// コンスタントバッファ作成
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(SIMPLESHADER_CONSTANT_BUFFER);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	g_p_device->CreateBuffer(&cb, nullptr, &g_p_constant_buffer);

	// 三角形作成
	SimpleVertex vertices[] =
	{
		D3DXVECTOR3(-0.5f, -0.5f, 0.0f),
		D3DXVECTOR3(-0.5f,  0.5f, 0.0f),
		D3DXVECTOR3( 0.5f, -0.5f, 0.0f)
	};
	D3D11_BUFFER_DESC bd;

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = vertices;
	g_p_device->CreateBuffer(&bd, &init_data, &g_p_vertex_buffer);

	return S_OK;
}

// ウィンドウプロシージャ関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0); break;
	default:
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

// メイン処理
void App()
{
	// レンダーターゲットビュー・デプスステンシルビューを設定
	g_p_device_context->OMSetRenderTargets(1, &g_p_rtv, g_p_dsv);
	
	/* 画面クリア処理 */
	// クリア色 (RGBA)
	const float clear_color[] = { 0.0f, 0.5f, 0.5f, 1.0f };
	
	// カラーバッファクリア
	g_p_device_context->ClearRenderTargetView(g_p_rtv, clear_color);
	// デプスステンシルバッファクリア
	g_p_device_context->ClearDepthStencilView(g_p_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	/* Rendering */
	D3DXMATRIX world, view, projection;
	D3DXMATRIX scale, translate, rotation;
	
	// Scale
	D3DXMatrixScaling(&scale, 2.0f, 1.0f, 1.0f);
	// Translate
	D3DXMatrixTranslation(&translate, 0.5f, 0.0f, 0.0f);
	// Rotation
	D3DXMatrixRotationY(&rotation, 3.14f/4.0f);
	// world transform
	D3DXMatrixIdentity(&world);
	XMMATRIX m_world = XMMatrixIdentity();

	world = scale * rotation * translate;

	// view transform
	D3DXVECTOR3 v_eye_pt(0.0f, 10.0f, -5.0f);
	D3DXVECTOR3 v_lookat_pt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 v_up_vec(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&view, &v_eye_pt, &v_lookat_pt, &v_up_vec);
	
	// projection transform
	D3DXMatrixPerspectiveFovLH(&projection, D3DX_PI / 4.0, static_cast<FLOAT>(w_width) / static_cast<FLOAT>(w_height), 0.1f, 100.0f);

	// シェーダの設定
	g_p_device_context->VSSetShader(g_p_vertex_shader, nullptr, 0);
	g_p_device_context->PSSetShader(g_p_pixel_shader, nullptr, 0);
	// シェーダのコンスタントバッファに各種データを渡す
	D3D11_MAPPED_SUBRESOURCE p_data;
	SIMPLESHADER_CONSTANT_BUFFER cb;

	if (SUCCEEDED(g_p_device_context->Map(g_p_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &p_data)))
	{
		// world, camera, projection 行列を渡す
		cb.m_wvp = world * view * projection;
		//cb.m_wvp = m_world * m_view * m_projection;
		D3DXMatrixTranspose(&cb.m_wvp, &cb.m_wvp);

		memcpy_s(p_data.pData, p_data.RowPitch, reinterpret_cast<void*>(&cb), sizeof(cb));
		g_p_device_context->Unmap(g_p_constant_buffer, 0);
	}

	// コンスタントバッファ使用時のシェーダを選択
	g_p_device_context->VSSetConstantBuffers(0, 1, &g_p_constant_buffer);
	g_p_device_context->PSSetConstantBuffers(0, 1, &g_p_constant_buffer);

	// VertexBufferを設定
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	g_p_device_context->IASetVertexBuffers(0, 1, &g_p_vertex_buffer, &stride, &offset);

	// VertexInputLayoutを設定
	g_p_device_context->IASetInputLayout(g_p_vertex_layout);
	// プリミティブ・トポロジを設定
	g_p_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// プリミティブをレンダリング
	g_p_device_context->Draw(3, 0);

	// 画面更新
	g_p_swap_chain->Present(0, 0);
}

// エントリーポイント
INT WINAPI WinMain(HINSTANCE h_inst, HINSTANCE h_prev_inst, LPSTR sz_str, INT i_cmd_show)
{
	/* ウィンドウ作成 */
	WNDCLASSEX wc = { 0 };

	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = h_inst;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	wc.lpszClassName = w_name;

	if (!RegisterClassEx(&wc)) { return 1; }

	g_hWnd = CreateWindow(w_name, w_name, WS_OVERLAPPEDWINDOW, 0, 0, w_width, w_height, 0, 0, h_inst, 0);

	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd);

	/* Direct3Dの初期化 */
	if (FAILED(InitDirect3D())) { return 1; }

	// メッセージループ
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { DispatchMessage(&msg); }
		else
		{
			App();
		}
	}

	return 0;
}