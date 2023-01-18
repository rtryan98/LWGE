#include "LWGE-RenderDriver/D3D12/D3D12CommandListRecycler.hpp"
#if LWGE_BUILD_D3D12

namespace lwge::rd::d3d12
{
    ComPtr<ID3D12CommandAllocator> create_command_allocator(
        NonOwningPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandAllocator> result = nullptr;
        device->CreateCommandAllocator(type, IID_PPV_ARGS(&result));
        return result;
    }

    D3D12CommandListRecycler::D3D12CommandListRecycler(NonOwningPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
        : m_device(device), m_allocator(create_command_allocator(m_device, type)),
        m_type(type), m_recycled(), m_used()
    {}

    NonOwningPtr<D3D12CmdList> D3D12CommandListRecycler::get_or_create_cmd_list() noexcept
    {
        NonOwningPtr<D3D12CmdList> result = nullptr;
        if (m_recycled.empty())
        {
            m_device->CreateCommandList(0, m_type, m_allocator.Get(), nullptr, IID_PPV_ARGS(&result));
        }
        else
        {
            result = m_recycled.back();
            m_recycled.pop_back();
        }
        m_used.push_back(result);
        return result;
    }

    void D3D12CommandListRecycler::reset() noexcept
    {
        m_allocator->Reset();
        m_recycled.insert(m_recycled.end(), m_used.begin(), m_used.end());
        m_used.clear();
    }
}
#endif // LWGE_BUILD_D3D12
