#pragma once

#include <cstdint>

namespace lwge::rd
{
    enum class Vendor : uint32_t;

    Vendor get_vendor_from_pci_id(uint32_t id);

    constexpr static uint32_t MAX_CONCURRENT_GPU_FRAMES = 2;
}
