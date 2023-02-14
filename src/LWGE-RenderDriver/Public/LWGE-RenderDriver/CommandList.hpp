#pragma once

#include "LWGE-RenderDriver/Resource.hpp"

#include <LWGE-Common/Pointer.hpp>
#include <array>
#include <cstdint>
#include <span>

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList7;

namespace lwge::rd
{
    class RenderDriver;
    class Swapchain;

    enum class IndexType
    {
        u16,
        u32
    };

    enum class SyncStage : uint64_t
    {
        None = 0x0,
        All = 0x1,
        Draw = 0x2,
        InputAssembler = 0x4,
        VertexShading = 0x8,
        PixelShading = 0x10,
        DepthStencil = 0x20,
        RenderTarget = 0x40,
        ComputeShading = 0x80,
        RayTracing = 0x100,
        Copy = 0x200,
        Resolve = 0x400,
        Indirect = 0x800,
        AllShading = 0x1000,
        NonPixelShading = 0x2000,
        EmitRayTracingAccelerationStructurePostBuildInfo = 0x4000,
        ClearUnorderedAccessView = 0x8000,
        BuildRayTracingAccelerationStructure = 0x800000,
        CopyRayTracingAccelerationStructure = 0x1000000,
        Split = 0x80000000
    };

    inline SyncStage operator|(SyncStage a, SyncStage b)
    {
        return static_cast<SyncStage>(static_cast<uint64_t>(a), static_cast<uint64_t>(b));
    }

    enum class Access : uint64_t
    {
        Common = 0,
        VertexBuffer = 0x1,
        IndexBuffer = 0x4,
        RenderTarget = 0x8,
        UnorderedAccess = 0x10,
        DepthStencilWrite = 0x20,
        DepthStencilRead = 0x40,
        ShaderResource = 0x80,
        Indirect = 0x200,
        CopyDest = 0x400,
        CopySource = 0x800,
        ResolveDest = 0x1000,
        ResolveSource = 0x2000,
        RaytracingAccelerationStructureRead = 0x4000,
        RaytracingAccelerationStructureWrite = 0x8000,
        ShadingRateSource = 0x10000,
        NoAccess = 0x80000000
    };

    inline Access operator|(Access a, Access b)
    {
        return static_cast<Access>(static_cast<uint64_t>(a), static_cast<uint64_t>(b));
    }

    enum class ImageLayout : uint64_t
    {
        Undefined = 0xffffffff,
        Common = 0,
        Present = 0,
        GenericRead = 1,
        RenderTarget = 2,
        UnorderedAccess = 3,
        DepthStencilWrite = 4,
        DepthStencilRead = 5,
        ShaderResource = 6,
        CopySource = 7,
        CopyDest = 8,
        ResolveSource = 9,
        ResolveDest = 10,
        ShadingRateImage = 11,
        DirectQueueCommon = 18,
        DirectQueueGenericRead = 19,
        DirectQueueUnorderedAccess = 20,
        DirectQueueShaderResource = 21,
        DirectQueueCopySource = 22,
        DirectQueueCopyDest = 23,
        ComputeQueueCommon = 24,
        ComputeQueueGenericRead = 25,
        ComputeQueueUnorderedAccess = 26,
        ComputeQueueShaderResource = 27,
        ComputeQueueCopySource = 28,
        ComputeQueueCopyDest = 29
    };

    struct MemoryBarrier
    {
        SyncStage sync_before;
        SyncStage sync_after;
        Access access_before;
        Access access_after;
    };

    struct BufferBarrier
    {
        SyncStage sync_before;
        SyncStage sync_after;
        Access access_before;
        Access access_after;
        BufferHandle buffer;
        const uint64_t offset = 0; // const until offset doesn't have to be 0 anymore
        uint64_t size = ~0ull;
    };

    struct BarrierSubresourceRange
    {
        uint32_t index_or_first_mip_level;
        uint32_t mip_level_count;
        uint32_t first_array_slice;
        uint32_t array_slice_count;
        uint32_t first_plane;
        uint32_t plane_count;
    };

    struct ImageBarrier
    {
        SyncStage sync_before;
        SyncStage sync_after;
        Access access_before;
        Access access_after;
        ImageLayout layout_before;
        ImageLayout layout_after;
        ImageHandle image;
        Swapchain* swapchain;
        uint32_t swapchain_image_index;
        BarrierSubresourceRange subresources;
        bool discard;
    };

    enum class BarrierType
    {
        Memory = 0,
        Image = 1,
        Buffer = 2
    };

    struct BarrierGroup
    {
        BarrierType type;
        std::span<MemoryBarrier> memory_barriers;
        std::span<BufferBarrier> buffer_barriers;
        std::span<ImageBarrier> image_barriers;
    };

    class CopyCommandList
    {
    public:
        CopyCommandList(ID3D12CommandAllocator& alloc,
            NonOwningPtr<ID3D12GraphicsCommandList7> cmd,
            NonOwningPtr<RenderDriver> driver);
        ~CopyCommandList() = default;

        [[nodiscard]] NonOwningPtr<ID3D12GraphicsCommandList7> get_d3d12_cmdlist() const noexcept
        {
            return m_cmd;
        }

        virtual void begin_recording() noexcept;
        void end_recording() noexcept;

        void barrier(const BarrierGroup& barrier_group) noexcept;
        void barrier(std::span<BarrierGroup> barrier_groups) noexcept;

    protected:
        NonOwningPtr<ID3D12CommandAllocator> m_alloc;
        NonOwningPtr<ID3D12GraphicsCommandList7> m_cmd;
        NonOwningPtr<RenderDriver> m_driver;
    };

    struct IndirectDispatchArgs
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };

    struct GpuVirtualAddressRange
    {
        GpuVirtualAddress start_address;
        uint64_t size;
    };

    struct GpuVirtualAddressStridedRange
    {
        GpuVirtualAddress start_address;
        uint64_t size;
        uint64_t stride;
    };

    struct IndirectDispatchRaysArgs
    {
        GpuVirtualAddressRange ray_gen_shader_record;
        GpuVirtualAddressStridedRange miss_shader_table;
        GpuVirtualAddressStridedRange hit_group_table;
        GpuVirtualAddressStridedRange callable_shader_table;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
    };

    class ComputeCommandList : public CopyCommandList
    {
    public:
        using CopyCommandList::CopyCommandList;
        ~ComputeCommandList() = default;

        virtual void begin_recording() noexcept override;

        void dispatch(uint32_t x, uint32_t y, uint32_t z) noexcept;
        void dispatch_indirect(BufferHandle arg_buf, uint64_t arg_offset) noexcept;
        // void dispatch_rays() noexcept;
        // void dispatch_rays_indirect() noexcept;
    };

    struct DrawIndirectArgs
    {
        uint32_t vertex_count;
        uint32_t instance_count;
        uint32_t first_vertex;
        uint32_t first_instance;
    };

    struct DrawIndexedIndirectArgs
    {
        uint32_t index_count;
        uint32_t instance_count;
        uint32_t first_index;
        uint32_t vertex_offset;
        uint32_t first_instance;
    };

    struct DispatchMeshIndirectArgs
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };

    class GraphicsCommandList : public ComputeCommandList
    {
    public:
        using ComputeCommandList::ComputeCommandList;
        ~GraphicsCommandList() = default;

        void clear_render_target(Swapchain* swapchain, uint32_t image_index,
            const std::array<float, 4>& rgba) noexcept;
        void clear_render_target(ImageHandle image,
            const std::array<float, 4>& rgba) noexcept;

        void draw(uint32_t vertex_count, uint32_t instance_count,
            uint32_t first_vertex, uint32_t first_instance) noexcept;
        void draw_indirect(BufferHandle arg_buf, uint64_t arg_offset,
            uint32_t draw_count) noexcept;
        void draw_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
            BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept;

        void draw_indexed(uint32_t index_count, uint32_t instance_count,
            uint32_t first_index, uint32_t first_instance,
            uint32_t vertex_offset) noexcept;
        void draw_indexed_indirect(BufferHandle arg_buf, uint64_t arg_offset,
            uint32_t draw_count) noexcept;
        void draw_indexed_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
            BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept;

        void dispatch_mesh(uint32_t x, uint32_t y, uint32_t z) noexcept;
        void dispatch_mesh_indirect(BufferHandle arg_buf, uint64_t arg_offset,
            uint32_t draw_count) noexcept;
        void dispatch_mesh_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
            BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept;

        void set_index_buffer(BufferHandle buf, uint32_t size, uint64_t offset, IndexType type) noexcept;
        void set_render_target(Swapchain* swapchain, uint32_t image_index, ImageHandle depth_stencil) noexcept;
        void set_render_target(Swapchain* swapchain, uint32_t image_index) noexcept;
        void set_render_targets(std::span<ImageHandle> color_targets, ImageHandle depth_stencil) noexcept;
        void set_render_targets(std::span<ImageHandle> color_targets) noexcept;

        void set_viewport(float_t x, float_t y, float_t width, float_t height,
            float_t min_depth, float_t max_depth) noexcept;
        void set_scissor(int32_t left, int32_t top, int32_t right, int32_t bottom) noexcept;
    };
}
