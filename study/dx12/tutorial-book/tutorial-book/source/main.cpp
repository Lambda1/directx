#include <Windows.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <iostream>

std::ostream& XM_CALLCONV operator<<(std::ostream& os, DirectX::FXMVECTOR v)
{
    DirectX::XMFLOAT3 dest;
    DirectX::XMStoreFloat3(&dest, v);
    
    os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ")";
    return os;
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    std::cout.setf(std::ios_base::boolalpha);

    // SSE2のサポートチェック
    if (!DirectX::XMVerifyCPUSupport()) { std::cout << "directx math not supported." << std::endl; return 0; }

    DirectX::XMVECTOR p = DirectX::XMVectorZero();
    DirectX::XMVECTOR q = DirectX::XMVectorSplatOne();
    DirectX::XMVECTOR u = DirectX::XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
    DirectX::XMVECTOR v = DirectX::XMVectorReplicate(-2.0f);
    DirectX::XMVECTOR w = DirectX::XMVectorSplatZ(u);

    std::cout << "p = " << p << std::endl;
    std::cout << "q = " << q << std::endl;
    std::cout << "u = " << u << std::endl;
    std::cout << "v = " << v << std::endl;
    std::cout << "w = " << w << std::endl;

    return 0;
}