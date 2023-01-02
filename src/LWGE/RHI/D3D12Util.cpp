#include "LWGE/RHI/D3D12Util.hpp"

#include <exception>

namespace lwge::rhi
{
	void throw_if_failed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}
}
