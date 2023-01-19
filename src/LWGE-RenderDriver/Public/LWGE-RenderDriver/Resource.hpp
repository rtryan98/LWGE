#pragma once

#include <LWGE-Common/Handle.hpp>

namespace lwge::rd
{
    enum class ResourceHeap
    {
        Vidmem,
        CPU,
        Readback,
    };

    struct BufferDesc
    {
        uint64_t size;
        ResourceHeap heap;
    };

    struct ImageDesc
    {

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
            struct
            {
                Shader amplification;
                Shader mesh;
                Shader pixel;
            } mesh_shaders;
        };

    };

    struct ComputePipelineDesc
    {

    };

    struct Buffer
    {};

    struct Image
    {};

    struct Pipeline
    {
        union
        {
            GraphicsPipelineDesc graphics_desc;
            ComputePipelineDesc compute_desc;
        };
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
