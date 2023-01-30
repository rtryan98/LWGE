#include "LWGE-RenderDriver/CommandList.hpp"
#include "LWGE-RenderDriver/CommandListRecycler.hpp"
#include "LWGE-RenderDriver/RenderDriver.hpp"

#include <array>

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

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

    void CopyCommandList::barrier(const BarrierGroup& barrier_group) noexcept
    {
        auto arr = std::to_array({ barrier_group });
        barrier({ arr });
    }

    void CopyCommandList::barrier(std::span<BarrierGroup> barrier_groups) noexcept
    {
        std::vector<D3D12_BARRIER_GROUP> groups = {};
        std::vector<std::vector<D3D12_GLOBAL_BARRIER>> global_barriers = {};
        std::vector<std::vector<D3D12_TEXTURE_BARRIER>> image_barriers = {};
        std::vector<std::vector<D3D12_BUFFER_BARRIER>> buffer_barriers = {};
        groups.reserve(barrier_groups.size());
        for (const auto& barrier_group : barrier_groups)
        {
            auto& barrier = groups.emplace_back();
            barrier.Type = D3D12_BARRIER_TYPE(barrier_group.type);
            switch (barrier_group.type)
            {
            case BarrierType::Memory:
            {
                barrier.NumBarriers = uint32_t(barrier_group.memory_barriers.size());
                auto& vec = global_barriers.emplace_back();
                vec.reserve(barrier_group.memory_barriers.size());
                for (const auto& global_barrier : barrier_group.memory_barriers)
                {
                    vec.push_back({
                        .SyncBefore = D3D12_BARRIER_SYNC(global_barrier.sync_before),
                        .SyncAfter = D3D12_BARRIER_SYNC(global_barrier.sync_after),
                        .AccessBefore = D3D12_BARRIER_ACCESS(global_barrier.access_before),
                        .AccessAfter = D3D12_BARRIER_ACCESS(global_barrier.access_after)
                        });
                }
                barrier.pGlobalBarriers = vec.data();
                break;
            }
            case BarrierType::Image:
            {
                barrier.NumBarriers = uint32_t(barrier_group.image_barriers.size());
                auto& vec = image_barriers.emplace_back();
                vec.reserve(barrier_group.image_barriers.size());
                for (const auto& image_barrier : barrier_group.image_barriers)
                {
                    ID3D12Resource* resource = nullptr;
                    if (image_barrier.swapchain)
                    {
                        resource = image_barrier.swapchain->get_buffer(image_barrier.swapchain_image_index);
                    }
                    else
                    {
                        const auto& image_info = m_driver->get_image_info(image_barrier.image);
                        resource = image_info.resource;
                    }
                    vec.push_back({
                        .SyncBefore = D3D12_BARRIER_SYNC(image_barrier.sync_before),
                        .SyncAfter = D3D12_BARRIER_SYNC(image_barrier.sync_after),
                        .AccessBefore = D3D12_BARRIER_ACCESS(image_barrier.access_before),
                        .AccessAfter = D3D12_BARRIER_ACCESS(image_barrier.access_after),
                        .LayoutBefore = D3D12_BARRIER_LAYOUT(image_barrier.layout_before),
                        .LayoutAfter = D3D12_BARRIER_LAYOUT(image_barrier.layout_after),
                        .pResource = resource,
                        .Subresources = {
                            .IndexOrFirstMipLevel = image_barrier.subresources.index_or_first_mip_level,
                            .NumMipLevels = image_barrier.subresources.mip_level_count,
                            .FirstArraySlice = image_barrier.subresources.first_array_slice,
                            .NumArraySlices = image_barrier.subresources.array_slice_count,
                            .FirstPlane = image_barrier.subresources.first_plane,
                            .NumPlanes = image_barrier.subresources.plane_count
                        },
                        .Flags = image_barrier.discard
                            ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD
                            : D3D12_TEXTURE_BARRIER_FLAG_NONE
                        });
                }
                barrier.pTextureBarriers = vec.data();
                break;
            }
            case BarrierType::Buffer:
            {
                barrier.NumBarriers = uint32_t(barrier_group.buffer_barriers.size());
                auto& vec = buffer_barriers.emplace_back();
                vec.reserve(barrier_group.buffer_barriers.size());
                for (const auto& buffer_barrier : barrier_group.buffer_barriers)
                {
                    const auto& buffer_info = m_driver->get_buffer_info(buffer_barrier.buffer);
                    vec.push_back({
                        .SyncBefore = D3D12_BARRIER_SYNC(buffer_barrier.sync_before),
                        .SyncAfter = D3D12_BARRIER_SYNC(buffer_barrier.sync_after),
                        .AccessBefore = D3D12_BARRIER_ACCESS(buffer_barrier.access_before),
                        .AccessAfter = D3D12_BARRIER_ACCESS(buffer_barrier.access_after),
                        .pResource = buffer_info.resource,
                        .Offset = buffer_barrier.offset,
                        .Size = buffer_barrier.size
                        });
                }
                barrier.pBufferBarriers = vec.data();
                break;
            }
            default:
                std::unreachable();
            }
        }
        m_cmd->Barrier(uint32_t(groups.size()), groups.data());
    }

    void ComputeCommandList::begin_recording() noexcept
    {
        CopyCommandList::begin_recording();
        std::array<ID3D12DescriptorHeap*, 2> heaps = {
            m_driver->get_cbv_srv_uav_descriptor_heap(),
            m_driver->get_sampler_descriptor_heap()
        };
        m_cmd->SetDescriptorHeaps(2, heaps.data());
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
