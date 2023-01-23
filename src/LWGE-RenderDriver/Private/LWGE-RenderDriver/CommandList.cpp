#include "LWGE-RenderDriver/CommandList.hpp"
#include "LWGE-RenderDriver/CommandListRecycler.hpp"

namespace lwge::rd
{
    CopyCommandList::CopyCommandList(ID3D12CommandAllocator& alloc,
        NonOwningPtr<ID3D12GraphicsCommandList7> cmd)
        : m_alloc(&alloc), m_cmd(cmd)
    {}

    void CopyCommandList::begin_recording() noexcept
    {
        m_cmd->Reset(m_alloc, nullptr);
    }

    void CopyCommandList::end_recording() noexcept
    {
        m_cmd->Close();
    }

    void CopyCommandList::barrier() noexcept
    {
    }

    void ComputeCommandList::dispatch(uint32_t x, uint32_t y, uint32_t z) noexcept
    {
        m_cmd->Dispatch(x, y, z);
    }

    void ComputeCommandList::dispatch_indirect(BufferHandle arg_buf, uint64_t arg_offset) noexcept
    {
        arg_buf, arg_offset;
    }

    void GraphicsCommandList::draw(uint32_t vertex_count, uint32_t instance_count,
        uint32_t first_vertex, uint32_t first_instance) noexcept
    {
        m_cmd->DrawInstanced(vertex_count, instance_count, first_vertex, first_instance);
    }

    void GraphicsCommandList::draw_indirect(BufferHandle arg_buf, uint64_t arg_offset, uint32_t draw_count) noexcept
    {
        arg_buf, arg_offset, draw_count;
    }

    void GraphicsCommandList::draw_indirect_count(BufferHandle arg_buf, uint64_t arg_offset, BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept
    {
        arg_buf, arg_offset, count_buf, count_offset, max_draws;
    }

    void GraphicsCommandList::draw_indexed(uint32_t index_count, uint32_t instance_count,
        uint32_t first_index, uint32_t first_instance, uint32_t vertex_offset) noexcept
    {
        m_cmd->DrawIndexedInstanced(index_count, instance_count, first_index,
            vertex_offset, first_instance);
    }

    void GraphicsCommandList::draw_indexed_indirect(BufferHandle arg_buf, uint64_t arg_offset, uint32_t draw_count) noexcept
    {
        arg_buf, arg_offset, draw_count;
    }

    void GraphicsCommandList::draw_indexed_indirect_count(BufferHandle arg_buf, uint64_t arg_offset, BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept
    {
        arg_buf, arg_offset, count_buf, count_offset, max_draws;
    }

    void GraphicsCommandList::dispatch_mesh(uint32_t x, uint32_t y, uint32_t z) noexcept
    {
        m_cmd->DispatchMesh(x, y, z);
    }

    void GraphicsCommandList::dispatch_mesh_indirect(BufferHandle arg_buf, uint64_t arg_offset, uint32_t draw_count) noexcept
    {
        arg_buf, arg_offset, draw_count;
    }

    void GraphicsCommandList::dispatch_mesh_indirect_count(BufferHandle arg_buf, uint64_t arg_offset, BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept
    {
        arg_buf, arg_offset, count_buf, count_offset, max_draws;
    }

    void GraphicsCommandList::set_index_buffer(BufferHandle buf, uint64_t offset, IndexType type) noexcept
    {
        buf, offset, type;
    }
}
