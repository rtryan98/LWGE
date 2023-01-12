#pragma once

namespace lwge::rhi
{
    enum class RenderDriverAPI
    {
        Null = 0,
        D3D12_12,
        Mock,
    };

    struct Frame;

    class RenderDriver
    {
    public:
        RenderDriver();
        virtual ~RenderDriver();

    private:

    };
}
