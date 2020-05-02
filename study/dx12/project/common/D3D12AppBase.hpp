#ifndef __D3D12_APP_BASE_HPP__
#define __D3D12_APP_BASE_HPP__

#include <Windows.h>
#include <wrl.h>

#include <d3d12.h>

namespace my_lib
{
	class D3D12AppBase
	{
	public:
		const UINT m_frame_buffer_count = 2;

		D3D12AppBase();
		virtual ~D3D12AppBase(){}

		void Initialize(HWND hWnd);
		void Terminate();

		virtual void Render(){}

		virtual void Prepare(){}
		virtual void CleanUp(){}
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& command){}

	protected:

	};
}

#endif
