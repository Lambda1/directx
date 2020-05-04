#ifndef __SIMPLE_TRIANGLE_HPP__
#define __SIMPLE_TRIANGLE_HPP__

#include "../common/D3D12AppBase.hpp"

#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace my_lib
{
	class SimpleTriangle : public D3D12AppBase
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource1> CreateBuffer(UINT buffer_size, const void* initial_data);

		Microsoft::WRL::ComPtr<ID3DBlob> m_vertex_shader, m_pixel_shader;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline;

		Microsoft::WRL::ComPtr<ID3D12Resource1> m_vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource1> m_index_buffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertex_buffer_view;
		D3D12_INDEX_BUFFER_VIEW m_index_buffer_view;
		UINT m_index_count;
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
