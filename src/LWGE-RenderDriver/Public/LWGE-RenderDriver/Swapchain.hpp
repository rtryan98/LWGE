#pragma once

#include <LWGE-Common/Pointer.hpp>

#include "LWGE-RenderDriver/Constants.hpp"

#include <array>
#include <cstdint>

typedef struct HWND__* HWND;
struct IDXGISwapChain4;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;

namespace lwge
{
    class Window;
}

namespace lwge::rd
{
    class RenderDriver;

    struct SwapchainDesc
    {
        bool vsync;
    };

    class Swapchain
    {
    public:
        ~Swapchain();

        Swapchain(const Swapchain& other) = delete;
        Swapchain(Swapchain&& other) = delete;
        Swapchain& operator=(const Swapchain& other) = delete;
        Swapchain& operator=(Swapchain&& other) = delete;

        [[nodiscard]] uint32_t acquire_next_image() noexcept;
        [[nodiscard]] uint64_t get_image_descriptor_address(uint32_t index) const noexcept;
        [[nodiscard]] NonOwningPtr<ID3D12Resource> get_buffer(uint32_t index) const noexcept
        {
            return m_buffers[index];
        };
        void try_resize() noexcept;
        void present() noexcept;

    protected:
        Swapchain(const SwapchainDesc& desc, RenderDriver& driver,
            const Window& window) noexcept;
        friend RenderDriver;

    private:
        void get_buffers() noexcept;
        void create_rtv_descriptors() noexcept;

    private:
        bool m_vsync_enabled;
        uint32_t m_flags = 0;
        NonOwningPtr<RenderDriver> m_driver;
        OwningPtr<IDXGISwapChain4> m_swapchain = {};
        std::array<OwningPtr<ID3D12Resource>, MAX_CONCURRENT_GPU_FRAMES> m_buffers;
        OwningPtr<ID3D12DescriptorHeap> m_descriptor_heap;
        uint64_t m_cpu_descriptor_heap_start_address;
        uint64_t m_cpu_descriptor_heap_increment;
        HWND m_hwnd = {};
    };
}
