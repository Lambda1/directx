#ifndef __MY_HELPER_HPP__
#define __MY_HELPER_HPP__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdexcept>

inline void ThrowIfFailed(const HRESULT hr)
{
	if (FAILED(hr)) { throw std::exception(); }
}

#endif