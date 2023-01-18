#pragma once

#include <LWGE-Common/Handle.hpp>

namespace lwge::rd
{
    struct BufferDesc
    {

    };

    struct ImageDesc
    {

    };

    struct GraphicsPipelineDesc
    {

    };

    struct ComputePipelineDesc
    {

    };

    struct Buffer
    {
        BufferDesc desc;
    };

    struct Image
    {
        ImageDesc desc;
    };

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
        uint32_t api : 4;
        uint32_t flags : 6;
        uint32_t gen : 22;
        uint32_t idx;

        bool operator==(HandleValueType other) const noexcept
        {
            return (api == other.api)
                && (flags == other.flags)
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
