#include "LWGE-RenderDriver/CommandList.hpp"
#include "LWGE-RenderDriver/CommandListRecycler.hpp"
#include "LWGE-RenderDriver/RenderDriver.hpp"

#include <array>

namespace lwge::rd
{
    CopyCommandList::CopyCommandList(ID3D12CommandAllocator& alloc,
        NonOwningPtr<ID3D12GraphicsCommandList7> cmd,
        NonOwningPtr<RenderDriver> driver)
        : m_alloc(&alloc), m_cmd(cmd), m_driver(driver)
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
        const auto& indirect = m_driver->internal_get_indirect_layouts();
        const auto& arg_buffer = m_driver->get_buffer_info(arg_buf);
        m_cmd->ExecuteIndirect(indirect.dispatch_indirect, 1, arg_buffer.resource, arg_offset, nullptr, 0);
    }

    void GraphicsCommandList::clear_render_target(Swapchain* swapchain, uint32_t image_index,
        const std::array<float, 4>& rgba) noexcept
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor_handle = {
            .ptr = swapchain->get_image_descriptor_address(image_index)
        };
        m_cmd->ClearRenderTargetView(cpu_descriptor_handle, rgba.data(), 0, nullptr);
    }

    void GraphicsCommandList::clear_render_target(ImageHandle image,
        const std::array<float, 4>& rgba) noexcept
    {
        image, rgba;
    }

    void GraphicsCommandList::draw(uint32_t vertex_count, uint32_t instance_count,
        uint32_t first_vertex, uint32_t first_instance) noexcept
    {
        m_cmd->DrawInstanced(vertex_count, instance_count, first_vertex, first_instance);
    }

    void GraphicsCommandList::draw_indirect(BufferHandle arg_buf, uint64_t arg_offset, uint32_t draw_count) noexcept
    {
        const auto& indirect = m_driver->internal_get_indirect_layouts();
        const auto& arg_buffer = m_driver->get_buffer_info(arg_buf);
        m_cmd->ExecuteIndirect(indirect.draw_indirect, draw_count, arg_buffer.resource, arg_offset, nullptr, 0);
    }

    void GraphicsCommandList::draw_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
        BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept
    {
        const auto& indirect = m_driver->internal_get_indirect_layouts();
        const auto& arg_buffer = m_driver->get_buffer_info(arg_buf);
        const auto& count_buffer = m_driver->get_buffer_info(count_buf);
        m_cmd->ExecuteIndirect(indirect.draw_indirect, max_draws, arg_buffer.resource, arg_offset,
            count_buffer.resource, count_offset);
    }

    void GraphicsCommandList::draw_indexed(uint32_t index_count, uint32_t instance_count,
        uint32_t first_index, uint32_t first_instance, uint32_t vertex_offset) noexcept
    {
        m_cmd->DrawIndexedInstanced(index_count, instance_count, first_index,
            vertex_offset, first_instance);
    }

    void GraphicsCommandList::draw_indexed_indirect(BufferHandle arg_buf, uint64_t arg_offset, uint32_t draw_count) noexcept
    {
        const auto& indirect = m_driver->internal_get_indirect_layouts();
        const auto& arg_buffer = m_driver->get_buffer_info(arg_buf);
        m_cmd->ExecuteIndirect(indirect.draw_indexed_indirect, draw_count, arg_buffer.resource, arg_offset, nullptr, 0);
    }

    void GraphicsCommandList::draw_indexed_indirect_count(BufferHandle arg_buf, uint64_t arg_offset, BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept
    {
        const auto& indirect = m_driver->internal_get_indirect_layouts();
        const auto& arg_buffer = m_driver->get_buffer_info(arg_buf);
        const auto& count_buffer = m_driver->get_buffer_info(count_buf);
        m_cmd->ExecuteIndirect(indirect.draw_indexed_indirect, max_draws, arg_buffer.resource, arg_offset,
            count_buffer.resource, count_offset);
    }

    void GraphicsCommandList::dispatch_mesh(uint32_t x, uint32_t y, uint32_t z) noexcept
    {
        m_cmd->DispatchMesh(x, y, z);
    }

    void GraphicsCommandList::dispatch_mesh_indirect(BufferHandle arg_buf, uint64_t arg_offset, uint32_t draw_count) noexcept
    {
        const auto& indirect = m_driver->internal_get_indirect_layouts();
        const auto& arg_buffer = m_driver->get_buffer_info(arg_buf);
        m_cmd->ExecuteIndirect(indirect.dispatch_mesh_indirect, draw_count, arg_buffer.resource, arg_offset, nullptr, 0);
    }

    void GraphicsCommandList::dispatch_mesh_indirect_count(BufferHandle arg_buf, uint64_t arg_offset, BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept
    {
        const auto& indirect = m_driver->internal_get_indirect_layouts();
        const auto& arg_buffer = m_driver->get_buffer_info(arg_buf);
        const auto& count_buffer = m_driver->get_buffer_info(count_buf);
        m_cmd->ExecuteIndirect(indirect.dispatch_mesh_indirect, max_draws, arg_buffer.resource, arg_offset,
            count_buffer.resource, count_offset);
    }

    inline DXGI_FORMAT index_type_to_format(IndexType type) noexcept
    {
        switch (type)
        {
        case IndexType::u16:
            return DXGI_FORMAT_R16_UINT;
        case IndexType::u32:
            return DXGI_FORMAT_R32_UINT;
        default:
            std::unreachable();
        }
    }

    void GraphicsCommandList::set_index_buffer(BufferHandle buf, uint32_t size, uint64_t offset, IndexType type) noexcept
    {
        const auto& buffer = m_driver->get_buffer_info(buf);
        D3D12_INDEX_BUFFER_VIEW view = {
            .BufferLocation = buffer.address + offset,
            .SizeInBytes = size,
            .Format = index_type_to_format(type)
        };
        m_cmd->IASetIndexBuffer(&view);
    }

    void GraphicsCommandList::set_render_target(Swapchain* swapchain, uint32_t image_index, ImageHandle depth_stencil) noexcept
    {
        depth_stencil;
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor_handle = {
            .ptr = swapchain->get_image_descriptor_address(image_index)
        };
        m_cmd->OMSetRenderTargets(1, &cpu_descriptor_handle, false, nullptr);
    }

    void GraphicsCommandList::set_render_target(Swapchain* swapchain, uint32_t image_index) noexcept
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor_handle = {
            .ptr = swapchain->get_image_descriptor_address(image_index)
        };
        m_cmd->OMSetRenderTargets(1, &cpu_descriptor_handle, false, nullptr);
    }

    void GraphicsCommandList::set_render_targets(std::span<ImageHandle> color_targets, ImageHandle depth_stencil) noexcept
    {
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> rts = {};
        for (auto color_target : color_targets)
        {
            const auto& img = m_driver->get_image_info(color_target);
            img;
            // TODO: implement
        }
        depth_stencil;
        D3D12_CPU_DESCRIPTOR_HANDLE ds = {};
        m_cmd->OMSetRenderTargets(uint32_t(color_targets.size()), rts.data(), false, &ds);
    }

    void GraphicsCommandList::set_render_targets(std::span<ImageHandle> color_targets) noexcept
    {
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> rts = {};
        for (auto color_target : color_targets)
        {
            const auto& img = m_driver->get_image_info(color_target);
            img;
            // TODO: implement
        }
        m_cmd->OMSetRenderTargets(uint32_t(color_targets.size()), rts.data(), false, nullptr);
    }
}
