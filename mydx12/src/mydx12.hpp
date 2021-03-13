#ifndef __MY_DX12_HPP__
#define __MY_DX12_HPP__

#include <wrl.h>

#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")

namespace mla
{
	class mydx12
	{
	public:
		mydx12();
		virtual ~mydx12();

		virtual void Init(HWND hwnd);
		void Terminate();

		virtual void Render();

		virtual void Prepare();
		virtual void CleanUp();
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& cmd);

		const UINT m_frame_buffer_count; // バッファリング数

	protected:
	private:
	};
}

#endif
