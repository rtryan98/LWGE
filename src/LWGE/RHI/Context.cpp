#include "LWGE/RHI/Context.hpp"
#include "LWGE/RHI/D3D12Util.hpp"
#include "LWGE/RHI/Swapchain.hpp"
#include "LWGE/Window/Window.hpp"

#include <atomic>

namespace lwge::rhi
{
	namespace detail
	{
		ComPtr<IDXGIFactory7> create_dxgi_factory()
		{
			uint32_t factory_flags = 0u;
#if LWGE_GRAPHICS_DEBUG
			factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
			ComPtr<ID3D12Debug> debug = nullptr;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
			{
				debug->EnableDebugLayer();
			}
#endif
			ComPtr<IDXGIFactory7> result = nullptr;
			throw_if_failed(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&result)));
			return result;
		}

		ComPtr<IDXGIAdapter4> create_adapter(IDXGIFactory7* factory)
		{
			ComPtr<IDXGIAdapter4> result = nullptr;
			throw_if_failed(factory->EnumAdapterByGpuPreference(0,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&result)));
			return result;
		}

		ComPtr<ID3D12Device10> create_device(IDXGIAdapter4* adapter)
		{
			ComPtr<ID3D12Device10> result = nullptr;
			throw_if_failed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&result)));
			return result;
		}

		ComPtr<ID3D12CommandQueue> create_command_queue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
		{
			D3D12_COMMAND_QUEUE_DESC desc = {
				.Type = type,
				.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
				.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
				.NodeMask = 0
			};
			ComPtr<ID3D12CommandQueue> result = nullptr;
			throw_if_failed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&result)));
			return result;
		}

		struct ContextPimpl
		{
			ContextPimpl(const Window& window, uint32_t thread_count)
				: thread_count(thread_count),
				factory(create_dxgi_factory()),
				adapter(create_adapter(factory.Get())),
				device(create_device(adapter.Get())),
				direct_queue(create_command_queue(device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT)),
				swapchain(factory.Get(), direct_queue.Get(), window.get_hwnd(), device.Get())
			{}

			const uint32_t thread_count;
			std::atomic<uint64_t> total_frames = 0;
			ComPtr<IDXGIFactory7> factory;
			ComPtr<IDXGIAdapter4> adapter;
			ComPtr<ID3D12Device10> device;
			ComPtr<ID3D12CommandQueue> direct_queue;
			Swapchain swapchain;
		};
	}

	Context::Context(const Window& window, uint32_t thread_count)
		: m_pimpl(new detail::ContextPimpl(window, thread_count))
	{}

	Context::~Context()
	{
		delete m_pimpl;
	}

	IDXGISwapChain* Context::get_swapchain() const
	{
		return m_pimpl->swapchain.get_dxgi_swapchain();
	}
}
