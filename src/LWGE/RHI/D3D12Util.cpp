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

	DWORD wait_for_fence(ID3D12Fence1* fence, uint64_t target_value, uint32_t timeout)
	{
		if (fence->GetCompletedValue() < target_value)
		{
			HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
			throw_if_failed(fence->SetEventOnCompletion(target_value, event));
			if (!event)
			{
				throw std::exception();
			}
			return WaitForSingleObject(event, timeout);
		}
		return WAIT_FAILED;
	}
}
