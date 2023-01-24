#pragma once

#include "LWGE-RenderDriver/Resource.hpp"

#include <LWGE-Common/Pointer.hpp>
#include <cstdint>
#include <span>

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList7;

namespace lwge::rd
{
    class RenderDriver;

    enum class IndexType
    {
        u16,
        u32
    };

    enum class SyncStage : uint64_t
    {
        None = 0x0,
        AllCommands = 0x1,
        AllGraphics = 0x2,
        VertexAttributeInput = 0x4,
        IndexInput = 0x8,
        VertexInput = VertexAttributeInput | IndexInput,
        VertexShader = 0x10,
        PixelShader = 0x20,
        GeometryShader = 0x40,
        HullShader = 0x80,
        DomainShader = 0x100,
        MeshShader = 0x200,
        AmplificationShader = 0x400,
        ColorAttachmentOutput = 0x800,
        EarlyFragmentTests = 0x1000,
        LateFragmentTests = 0x2000,
        ComputeShader = 0x4000,
        RayTracingShader = 0x8000,
        Copy = 0x10000,
        Blit = 0x20000,
        Resolve = 0x40000,
        AccelerationStructureBuild = 0x80000,
        AccelerationStructureCopy = 0x100000,
        AllTransfer = 0x200000,
        Indirect = 0x400000,
        AllShaders = 0x800000,
        PreRasterShaders = 0x1000000,
        VideoDecode = 0x2000000,
        VideoEncode = 0x4000000,
        VideoProcess = 0x8000000
    };

    enum class Access : uint64_t
    {

    };

    struct MemoryBarrier
    {

    };

    struct BufferBarrier
    {

    };

    struct ImageBarrier
    {

    };

    struct Barrier
    {
        std::span<MemoryBarrier> memory_barriers;
        std::span<BufferBarrier> buffer_barriers;
        std::span<ImageBarrier> image_barriers;
    };

    class CopyCommandList
    {
    public:
        CopyCommandList(ID3D12CommandAllocator& alloc,
            NonOwningPtr<ID3D12GraphicsCommandList7> cmd);
        ~CopyCommandList() = default;

        void begin_recording() noexcept;
        void end_recording() noexcept;

        void barrier() noexcept;

    protected:
        NonOwningPtr<ID3D12CommandAllocator> m_alloc;
        NonOwningPtr<ID3D12GraphicsCommandList7> m_cmd;
    };

    struct IndirectDispatchArgs
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };

    using GpuVirtualAddress = uint64_t;

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

        void begin_render_pass() noexcept;
        void end_render_pass() noexcept;

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

        void set_index_buffer(BufferHandle buf, uint64_t offset, IndexType type) noexcept;
    };
}
