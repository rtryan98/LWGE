#include "LWGE/RHI/CommandListRecycler.hpp"

namespace lwge::rhi
{
    ComPtr<ID3D12CommandAllocator> create_command_allocator(
        ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandAllocator> result = nullptr;
        device->CreateCommandAllocator(type, IID_PPV_ARGS(&result));
        return result;
    }

    CommandListRecycler::CommandListRecycler(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
        : m_device(device), m_allocator(create_command_allocator(m_device, type)),
        m_type(type), m_recycled(), m_used()
    {}

    D3D12CmdList* CommandListRecycler::get_or_create_cmd_list()
    {
        D3D12CmdList* result = nullptr;
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

    void CommandListRecycler::reset()
    {
        m_allocator->Reset();
        m_recycled.insert(m_recycled.end(), m_used.begin(), m_used.end());
        m_used.clear();
    }
}
