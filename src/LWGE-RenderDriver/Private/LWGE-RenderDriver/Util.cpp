#include "LWGE-RenderDriver/Util.hpp"
#include "LWGE-RenderDriver/RenderDriver.hpp"

#include "LWGE-RenderDriver/D3D12Includes.hpp"
#include <exception>

namespace lwge::rd
{
    Vendor get_vendor_from_pci_id(uint32_t id)
    {
        switch (id)
        {
        case 0x1002:
            return Vendor::AMD;
        case 0x10de:
            return Vendor::NVIDIA;
        case 0x8086:
            return Vendor::INTEL;
        default:
            return Vendor::Unknown;
        }
    }

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
