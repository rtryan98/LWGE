#pragma once

#include <LWGE-Common/Handle.hpp>
#include <LWGE-Common/Pointer.hpp>
#include <LWGE-ImageFormat/Format.hpp>

#include <array>

struct ID3D12Resource2;
struct ID3D12PipelineState;

namespace lwge::rd
{
    using GpuVirtualAddress = uint64_t;

    enum class ResourceHeap
    {
        Vidmem = 1,
        CPU = 2,
        Readback = 3,
    };

    struct BufferDesc
    {
        uint64_t size;
        ResourceHeap heap;
    };

    enum class ImageDimension
    {
        Unknown = 0,
        Tex1D = 2,
        Tex1DArray = 3,
        Tex2D = 4,
        Tex2DArray = 5,
        Tex3D = 8,
        TexCube = 9,
        TexCubeArray = 10
    };

    struct ImageDesc
    {
        uint16_t width;
        uint16_t height;
        uint16_t depth;
        uint16_t mip_levels;
        ImageDimension dim_type;
        Format format;
    };

    struct Shader
    {
        uint64_t size;
        void* bytecode;
    };

    enum class PipelineType
    {
        Graphics,
        Compute,
        MeshShader,
        RayTracing
    };

    enum class PrimitiveTopology
    {
        Point = 1,
        Line = 2,
        Triangle = 3,
        Patch = 4
    };

    enum class StencilOp
    {
        Keep = 1,
        Zero = 2,
        Replace = 3,
        IncrSat = 4,
        DecrSat = 5,
        Invert = 6,
        Incr = 7,
        Decr = 8
    };

    enum class CompareFunc
    {
        None = 0,
        Never = 1,
        Less = 2,
        Equal = 3,
        LessEqual = 4,
        Greater = 5,
        NotEqual = 6,
        GreaterEqual = 7,
        Always = 8
    };

    struct GraphicsPipelineDesc
    {
        PipelineType type;
        union
        {
            struct
            {
                Shader vertex;
                Shader domain;
                Shader hull;
                Shader geometry;
                Shader pixel;
            } graphics_shaders;
        };
        PrimitiveTopology topology;
        struct DepthStencilState
        {
            struct StencilOps
            {
                StencilOp fail;
                StencilOp depth_fail;
                StencilOp pass;
                CompareFunc func;
            };

            Format ds_format;
            bool depth_enable;
            bool enable_depth_write;
            CompareFunc depth_compare_func;
            bool stencil_enable;
            uint8_t stencil_read_mask;
            uint8_t stencil_write_mask;
            StencilOps stencil_front_face;
            StencilOps stencil_back_face;
        } ds_state;
        uint32_t render_target_count;
        std::array<Format, 8> render_target_formats;
    };

    struct ComputePipelineDesc
    {
        Shader shader;
    };

    struct Buffer
    {
        uint32_t srv_idx;
        uint32_t uav_idx;
        GpuVirtualAddress address;
        OwningPtr<ID3D12Resource2> resource;
    };

    struct Image
    {
        uint16_t width;
        uint16_t height;
        uint16_t depth;
        Format format;
        uint32_t srv_idx;
        uint32_t uav_idx;
        uint64_t cpu_rt_ds_descriptor_address;
        OwningPtr<ID3D12Resource2> resource;
    };

    struct Pipeline
    {
        OwningPtr<ID3D12PipelineState> resource;
    };

    struct HandleValueType
    {
        uint16_t flags;
        uint16_t gen;
        uint32_t idx;

        bool operator==(HandleValueType other) const noexcept
        {
            return (flags == other.flags)
                && (gen == other.gen)
                && (idx == other.idx);
        }
        bool operator!=(HandleValueType other) const noexcept
        {
            return !(*this == other);
        }
    };

    using BufferHandle = Handle<Buffer, HandleValueType>;
    using ImageHandle = Handle<Image, HandleValueType>;
    using PipelineHandle = Handle<Pipeline, HandleValueType>;

    static_assert(sizeof(BufferHandle) == sizeof(void*));
}
