#pragma once

#include <vector>

#include "LWGE/RHI/D3D12Util.hpp"

namespace lwge::rhi
{
    using D3D12CmdList = ID3D12GraphicsCommandList9;

    class CommandListRecycler
    {
    public:
        CommandListRecycler(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);

        D3D12CmdList* get_or_create_cmd_list();
        void reset();

    private:
        ID3D12Device* m_device;
        ComPtr<ID3D12CommandAllocator> m_allocator;
        D3D12_COMMAND_LIST_TYPE m_type;
        std::vector<D3D12CmdList*> m_recycled;
        std::vector<D3D12CmdList*> m_used;
    };
}
