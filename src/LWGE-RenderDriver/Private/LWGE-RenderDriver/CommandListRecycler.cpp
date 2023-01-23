#include "LWGE-RenderDriver/CommandListRecycler.hpp"

namespace lwge::rd
{
    ComPtr<ID3D12CommandAllocator> create_command_allocator(
        NonOwningPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandAllocator> result = nullptr;
        device->CreateCommandAllocator(type, IID_PPV_ARGS(&result));
        return result;
    }

    CommandListRecycler::CommandListRecycler(NonOwningPtr<ID3D12Device4> device, D3D12_COMMAND_LIST_TYPE type)
        : m_device(device), m_allocator(create_command_allocator(m_device, type)),
        m_type(type), m_recycled(), m_used()
    {}

    CommandListRecycler::~CommandListRecycler()
    {
        m_recycled.insert(m_recycled.end(), m_used.begin(), m_used.end());
        m_used.clear();
        for (auto& cmd : m_recycled)
        {
            cmd->Release();
        }
    }

    NonOwningPtr<CmdListType> CommandListRecycler::get_or_create_cmd_list() noexcept
    {
        NonOwningPtr<CmdListType> result = nullptr;
        if (m_recycled.empty())
        {
            m_device->CreateCommandList1(0, m_type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&result));
        }
        else
        {
            result = m_recycled.back();
            m_recycled.pop_back();
        }
        m_used.push_back(result);
        return result;
    }

    void CommandListRecycler::reset() noexcept
    {
        m_allocator->Reset();
        m_recycled.insert(m_recycled.end(), m_used.begin(), m_used.end());
        m_used.clear();
    }
}