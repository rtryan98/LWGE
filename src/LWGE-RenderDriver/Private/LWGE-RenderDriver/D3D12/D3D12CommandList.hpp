#pragma once
#if LWGE_BUILD_D3D12

#include "LWGE-RenderDriver/CommandList.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Includes.hpp"

namespace lwge::rd::d3d12
{
    class D3D12CommandList : public GraphicsCommandList
    {
    public:


    private:
        ID3D12GraphicsCommandList9* m_cmd;
    };
}

#endif // LWGE_BUILD_D3D12
