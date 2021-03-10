#ifndef __VERTEX_HPP__
#define __VERTEX_HPP__

#include <DirectXMath.h>

namespace mla
{
	struct Vertex
	{
		DirectX::XMFLOAT3 m_pos;
		DirectX::XMFLOAT2 m_uv;
	};
}

#endif
