#pragma once
#if LWGE_BUILD_D3D12

#include "LWGE-Common/Pointer.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Includes.hpp"
#include <vector>

namespace lwge::rd::d3d12
{
    using D3D12CmdListType = ID3D12GraphicsCommandList7;

    class D3D12CommandListRecycler
    {
    public:
        D3D12CommandListRecycler(NonOwningPtr<ID3D12Device4> device, D3D12_COMMAND_LIST_TYPE type);
        ~D3D12CommandListRecycler();

        [[nodiscard]] NonOwningPtr<D3D12CmdListType> get_or_create_cmd_list() noexcept;
        [[nodiscard]] NonOwningPtr<ID3D12CommandAllocator> get_allocator() const noexcept { return m_allocator.Get(); }
        void reset() noexcept;

    private:
        NonOwningPtr<ID3D12Device4> m_device;
        ComPtr<ID3D12CommandAllocator> m_allocator;
        D3D12_COMMAND_LIST_TYPE m_type;
        std::vector<OwningPtr<D3D12CmdListType>> m_recycled;
        std::vector<OwningPtr<D3D12CmdListType>> m_used;
    };
}

#endif
