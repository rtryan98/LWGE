#pragma once

#include "LWGE-Common/Pointer.hpp"
#include "LWGE-RenderDriver/D3D12Includes.hpp"
#include <vector>

namespace lwge::rd
{
    using CmdListType = ID3D12GraphicsCommandList7;

    class CommandListRecycler
    {
    public:
        CommandListRecycler(NonOwningPtr<ID3D12Device4> device, D3D12_COMMAND_LIST_TYPE type);
        ~CommandListRecycler();

        [[nodiscard]] NonOwningPtr<CmdListType> get_or_create_cmd_list() noexcept;
        [[nodiscard]] NonOwningPtr<ID3D12CommandAllocator> get_allocator() const noexcept { return m_allocator.Get(); }
        void reset() noexcept;

    private:
        NonOwningPtr<ID3D12Device4> m_device;
        ComPtr<ID3D12CommandAllocator> m_allocator;
        D3D12_COMMAND_LIST_TYPE m_type;
        std::vector<OwningPtr<CmdListType>> m_recycled;
        std::vector<OwningPtr<CmdListType>> m_used;
    };
}
