#include "LWGE-RenderDriver/Util.hpp"
#include "LWGE-RenderDriver/RenderDriver.hpp"

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
}
