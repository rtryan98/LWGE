#pragma once
#if LWGE_BUILD_D3D12

#include "LWGE-RenderDriver/D3D12/D3D12CommandList.hpp"

namespace lwge::rd::d3d12
{
    constexpr static [[nodiscard]] D3D12_BARRIER_SYNC translate_syncstage(SyncStage sync) noexcept
    {
        switch (sync)
        {
        case lwge::rd::SyncStage::None:
            return D3D12_BARRIER_SYNC_NONE;
        case lwge::rd::SyncStage::AllCommands:
            return D3D12_BARRIER_SYNC_ALL;
        case lwge::rd::SyncStage::AllGraphics:
            return D3D12_BARRIER_SYNC_ALL_SHADING
                | D3D12_BARRIER_SYNC_RENDER_TARGET
                | D3D12_BARRIER_SYNC_DEPTH_STENCIL;
        case lwge::rd::SyncStage::VertexAttributeInput:
            return D3D12_BARRIER_SYNC_VERTEX_SHADING;
        case lwge::rd::SyncStage::IndexInput:
            return D3D12_BARRIER_SYNC_VERTEX_SHADING;
        case lwge::rd::SyncStage::VertexInput:
            return D3D12_BARRIER_SYNC_VERTEX_SHADING;
        case lwge::rd::SyncStage::VertexShader:
            return D3D12_BARRIER_SYNC_VERTEX_SHADING;
        case lwge::rd::SyncStage::PixelShader:
            return D3D12_BARRIER_SYNC_PIXEL_SHADING;
        case lwge::rd::SyncStage::GeometryShader:
            return D3D12_BARRIER_SYNC_VERTEX_SHADING;
        case lwge::rd::SyncStage::HullShader:
            return D3D12_BARRIER_SYNC_VERTEX_SHADING;
        case lwge::rd::SyncStage::DomainShader:
            return D3D12_BARRIER_SYNC_VERTEX_SHADING;
        case lwge::rd::SyncStage::MeshShader:
            return D3D12_BARRIER_SYNC_VERTEX_SHADING;
        case lwge::rd::SyncStage::AmplificationShader:
            return D3D12_BARRIER_SYNC_VERTEX_SHADING;
        case lwge::rd::SyncStage::ColorAttachmentOutput:
            return D3D12_BARRIER_SYNC_RENDER_TARGET;
        case lwge::rd::SyncStage::EarlyFragmentTests:
            return D3D12_BARRIER_SYNC_DEPTH_STENCIL;
        case lwge::rd::SyncStage::LateFragmentTests:
            return D3D12_BARRIER_SYNC_DEPTH_STENCIL;
        case lwge::rd::SyncStage::ComputeShader:
            return D3D12_BARRIER_SYNC_COMPUTE_SHADING;
        case lwge::rd::SyncStage::RayTracingShader:
            return D3D12_BARRIER_SYNC_RAYTRACING;
        case lwge::rd::SyncStage::Copy:
            return D3D12_BARRIER_SYNC_COPY;
        case lwge::rd::SyncStage::Blit:
            return D3D12_BARRIER_SYNC_RENDER_TARGET; // TODO: check if correct
        case lwge::rd::SyncStage::Resolve:
            return D3D12_BARRIER_SYNC_RESOLVE;
        case lwge::rd::SyncStage::AccelerationStructureBuild:
            return D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
        case lwge::rd::SyncStage::AccelerationStructureCopy:
            return D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE;
        case lwge::rd::SyncStage::AllTransfer:
            return D3D12_BARRIER_SYNC_COPY;
        case lwge::rd::SyncStage::Indirect:
            return D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
        case lwge::rd::SyncStage::AllShaders:
            return D3D12_BARRIER_SYNC_ALL_SHADING;
        case lwge::rd::SyncStage::PreRasterShaders:
            return D3D12_BARRIER_SYNC_NON_PIXEL_SHADING;
        case lwge::rd::SyncStage::VideoDecode:
            return D3D12_BARRIER_SYNC_VIDEO_DECODE;
        case lwge::rd::SyncStage::VideoEncode:
            return D3D12_BARRIER_SYNC_VIDEO_ENCODE;
        case lwge::rd::SyncStage::VideoProcess:
            return D3D12_BARRIER_SYNC_VIDEO_PROCESS;
        default:
            std::unreachable();
        }
    }

    D3D12CommandList::D3D12CommandList(NonOwningPtr<D3D12RenderDriver> driver,
        NonOwningPtr<ID3D12CommandAllocator> alloc,
        NonOwningPtr<ID3D12GraphicsCommandList9> cmd)
        : m_driver(driver), m_allocator(alloc), m_cmd(cmd)
    {
    }

    void D3D12CommandList::begin_recording() noexcept
    {
        m_cmd->Reset(m_allocator, nullptr);
    }

    void D3D12CommandList::end_recording() noexcept
    {
        m_cmd->Close();
    }

    void D3D12CommandList::end_render_pass() noexcept
    {
        m_cmd->EndRenderPass();
    }

    void D3D12CommandList::dispatch(uint32_t x, uint32_t y, uint32_t z) noexcept
    {
        m_cmd->Dispatch(x, y, z);
    }

    void D3D12CommandList::draw(uint32_t vertex_count, uint32_t instance_count,
        uint32_t first_vertex, uint32_t first_instance) noexcept
    {
        m_cmd->DrawInstanced(vertex_count, instance_count, first_vertex, first_instance);
    }

    void D3D12CommandList::draw_indexed(uint32_t index_count, uint32_t instance_count,
        uint32_t first_index, uint32_t first_instance, uint32_t vertex_offset) noexcept
    {
        m_cmd->DrawIndexedInstanced(index_count, instance_count, first_index,
            vertex_offset, first_instance);
    }

    void D3D12CommandList::dispatch_mesh(uint32_t x, uint32_t y, uint32_t z) noexcept
    {
        m_cmd->DispatchMesh(x, y, z);
    }
}

#endif // LWGE_BUILD_D3D12
