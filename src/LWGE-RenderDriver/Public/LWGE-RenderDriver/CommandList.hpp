#pragma once

#include "LWGE-RenderDriver/Resource.hpp"

#include <LWGE-Common/Pointer.hpp>
#include <cstdint>
#include <span>

namespace lwge::rd
{
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
        virtual ~CopyCommandList() = default;

        virtual void begin_recording() noexcept = 0;
        virtual void end_recording() noexcept = 0;

        virtual void barrier() noexcept = 0;
    };

    class ComputeCommandList : public virtual CopyCommandList
    {
    public:
        virtual ~ComputeCommandList() = default;

        virtual void dispatch(uint32_t x, uint32_t y, uint32_t z) noexcept = 0;
        virtual void dispatch_indirect(BufferHandle arg_buf, uint64_t arg_offset) noexcept = 0;
        // virtual void dispatch_rays() noexcept = 0;
        // virtual void dispatch_rays_indirect() noexcept = 0;
    };

    class GraphicsCommandList : public virtual ComputeCommandList
    {
    public:
        virtual ~GraphicsCommandList() = default;

        virtual void begin_render_pass() noexcept = 0;
        virtual void end_render_pass() noexcept = 0;

        virtual void draw(uint32_t vertex_count, uint32_t instance_count,
            uint32_t first_vertex = 0, uint32_t first_instance = 0) noexcept = 0;
        virtual void draw_indirect(BufferHandle arg_buf, uint64_t arg_offset,
            uint32_t draw_count) noexcept = 0;
        virtual void draw_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
            BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept = 0;

        virtual void draw_indexed(uint32_t index_count, uint32_t instance_count,
            uint32_t first_index = 0, uint32_t first_instance = 0,
            uint32_t vertex_offset = 0) noexcept = 0;
        virtual void draw_indexed_indirect(BufferHandle arg_buf, uint64_t arg_offset,
            uint32_t draw_count) noexcept = 0;
        virtual void draw_indexed_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
            BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept = 0;

        virtual void dispatch_mesh(uint32_t x, uint32_t y, uint32_t z) noexcept = 0;
        virtual void dispatch_mesh_indirect(BufferHandle arg_buf, uint64_t arg_offset,
            uint32_t draw_count) noexcept = 0;
        virtual void dispatch_mesh_indirect_count(BufferHandle arg_buf, uint64_t arg_offset,
            BufferHandle count_buf, uint64_t count_offset, uint32_t max_draws) noexcept = 0;

        virtual void set_index_buffer(BufferHandle buf, uint64_t offset, IndexType type) noexcept = 0;
    };
}
