#pragma once

#include "LWGE-Common/Pointer.hpp"

#include <cstdint>

namespace lwge::rd
{
    class RenderDriver;

    struct CommandListToken
    {
        uint16_t flags;
        uint16_t idx;
        uint32_t thread_idx;

    private:
        /// @brief Private Constructor so it's only constructible by RenderDriver.
        CommandListToken() noexcept;
        friend class RenderDriver;
    };

    class CopyCommandList
    {
    public:
        CopyCommandList(CommandListToken token, NonOwningPtr<RenderDriver> driver) noexcept
            : m_token(token), m_driver(driver) {}
        virtual ~CopyCommandList() = 0;

    protected:
        CommandListToken m_token;
        NonOwningPtr<RenderDriver> m_driver;
    };

    class ComputeCommandList : public CopyCommandList
    {
    public:
        using CopyCommandList::CopyCommandList;

        virtual void dispatch(uint32_t x, uint32_t y, uint32_t z) noexcept = 0;
        // virtual void dispatch_indirect() noexcept = 0;
        // virtual void dispatch_rays() noexcept = 0;
        // virtual void dispatch_rays_indirect() noexcept = 0;
    };

    class GraphicsCommandList : public ComputeCommandList
    {
    public:
        using ComputeCommandList::ComputeCommandList;

        virtual void draw(uint32_t vertex_count, uint32_t instance_count,
            uint32_t first_vertex = 0, uint32_t first_instance = 0) noexcept = 0;
        // virtual void draw_indirect() noexcept = 0;
        // virtual void draw_indirect_count() noexcept = 0;
        virtual void draw_indexed(uint32_t index_count, uint32_t instance_count,
            uint32_t first_index = 0, uint32_t first_instance = 0,
            uint32_t vertex_offset = 0) noexcept = 0;
        // virtual void draw_indexed_indirect() noexcept = 0;
        // virtual void draw_indexed_indirect_count() noexcept = 0;
        virtual void dispatch_mesh(uint32_t x, uint32_t y, uint32_t z) noexcept = 0;
        // virtual void dispatch_mesh_indirect() noexcept = 0;
        // virtual void dispatch_mesh_indirect_count() noexcept = 0;
    };
}
