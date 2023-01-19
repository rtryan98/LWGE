#pragma once
#if LWGE_BUILD_D3D12

#include "LWGE-RenderDriver/CommandList.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Includes.hpp"

namespace lwge::rd::d3d12
{
    class D3D12RenderDriver;

    class D3D12CommandList final : public GraphicsCommandList
    {
    public:
        D3D12CommandList(NonOwningPtr<D3D12RenderDriver> driver,
            NonOwningPtr<ID3D12CommandAllocator> alloc,
            NonOwningPtr<ID3D12GraphicsCommandList7> cmd);
        virtual ~D3D12CommandList() override = default;

        virtual void begin_recording() noexcept override;
        virtual void end_recording() noexcept override;

        virtual void barrier() noexcept override;

        virtual void begin_render_pass() noexcept override;
        virtual void end_render_pass() noexcept override;

        virtual void dispatch(uint32_t x, uint32_t y, uint32_t z) noexcept override;
        virtual void dispatch_indirect(BufferHandle arg_buf, uint64_t arg_offset) noexcept override;

        virtual void draw(uint32_t vertex_count, uint32_t instance_count,
            uint32_t first_vertex = 0, uint32_t first_instance = 0) noexcept override;
        virtual void draw_indirect(BufferHandle arg_buf, uint64_t arg_offset,
            uint32_t draw_count) noexcept override;
        virtual void draw_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
            BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept override;

        virtual void draw_indexed(uint32_t index_count, uint32_t instance_count,
            uint32_t first_index = 0, uint32_t first_instance = 0,
            uint32_t vertex_offset = 0) noexcept override;
        virtual void draw_indexed_indirect(BufferHandle arg_buf, uint64_t arg_offset,
            uint32_t draw_count) noexcept override;
        virtual void draw_indexed_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
            BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept override;

        virtual void dispatch_mesh(uint32_t x, uint32_t y, uint32_t z) noexcept override;
        virtual void dispatch_mesh_indirect(BufferHandle arg_buf, uint64_t arg_offset,
            uint32_t draw_count) noexcept override;
        virtual void dispatch_mesh_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
            BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept override;

        virtual void set_index_buffer(BufferHandle buf, uint64_t offset, IndexType type) noexcept override;

    private:
        NonOwningPtr<D3D12RenderDriver> m_driver;
        NonOwningPtr<ID3D12CommandAllocator> m_allocator;
        NonOwningPtr<ID3D12GraphicsCommandList7> m_cmd;
    };
}

#endif // LWGE_BUILD_D3D12
