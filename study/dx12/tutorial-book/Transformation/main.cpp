#include <Windows.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <iostream>

std::ostream& XM_CALLCONV operator<<(std::ostream& os, DirectX::FXMVECTOR v)
{
    DirectX::XMFLOAT4 dest;
    DirectX::XMStoreFloat4(&dest, v);
    
    os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ", " << dest.w <<  ")";
    return os;
}

std::ostream& XM_CALLCONV operator<<(std::ostream& os, DirectX::FXMMATRIX m)
{
    for (int i = 0; i < 4; ++i)
    {
        os << DirectX::XMVectorGetX(m.r[i]) << "\t" << DirectX::XMVectorGetY(m.r[i]) << "\t";
        os << DirectX::XMVectorGetZ(m.r[i]) << "\t" << DirectX::XMVectorGetW(m.r[i]) << std::endl;
    }
    return os;
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    std::cout.setf(std::ios_base::boolalpha);

    // SSE2のサポートチェック
    if (!DirectX::XMVerifyCPUSupport()) { std::cout << "directx math not supported." << std::endl; return 0; }

    // ベクトル
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

    // 行列
    DirectX::XMMATRIX A
    (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 4.0f, 0.0f,
        1.0f, 2.0f, 3.0f, 1.0f
    );
    DirectX::XMMATRIX B = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX C = A * B;
    DirectX::XMMATRIX D = DirectX::XMMatrixTranspose(A);
    DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
    DirectX::XMMATRIX E = DirectX::XMMatrixInverse(&det, A);
    DirectX::XMMATRIX F = A * E;

    std::cout << "A = " << std::endl << A << std::endl;
    std::cout << "B = " << std::endl << B << std::endl;
    std::cout << "C = " << std::endl << C << std::endl;
    std::cout << "D = " << std::endl << D << std::endl;
    std::cout << "E = " << std::endl << E << std::endl;
    std::cout << "F = " << std::endl << F << std::endl;
    std::cout << "det = " << std::endl << det << std::endl;

    return 0;
}