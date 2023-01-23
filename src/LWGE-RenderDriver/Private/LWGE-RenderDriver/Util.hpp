#pragma once

#include <cstdint>

typedef long HRESULT;
typedef unsigned long DWORD;
struct ID3D12Fence1;

namespace lwge::rd
{
    enum class Vendor : uint32_t;

    Vendor get_vendor_from_pci_id(uint32_t id);

    void throw_if_failed(HRESULT hr);
    DWORD wait_for_fence(ID3D12Fence1* fence, uint64_t target_value, uint32_t timeout);
}
