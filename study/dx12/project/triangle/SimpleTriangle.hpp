#ifndef __SIMPLE_TRIANGLE_HPP__
#define __SIMPLE_TRIANGLE_HPP__

#include "../common/D3D12AppBase.hpp"

#include <DirectXMath.h>

namespace my_lib
{
	class SimpleTriangle : public D3D12AppBase
	{
	private:
		Microsoft::WRL::ComPtr<ID3DBlob> m_vertex_shader, m_pixel_shader;
	public:
		SimpleTriangle() : D3D12AppBase() {}

		virtual void Prepare() override;
		virtual void CleanUp() override;
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& command) override;

		struct Vertex
		{
			DirectX::XMFLOAT3 m_position;
			DirectX::XMFLOAT4 m_color;
		};
	};
};

#endif
