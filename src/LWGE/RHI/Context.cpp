#include "LWGE/RHI/Context.hpp"
#include "LWGE/Window/Window.hpp"

extern "C"
{
	__declspec(dllexport) extern const uint32_t D3D12SDKVersion = 608;
	__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

namespace lwge::rhi
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

	ComPtr<ID3D12Device10> create_device(IDXGIAdapter4* adapter, D3D_FEATURE_LEVEL feature_level)
	{
		ComPtr<ID3D12Device10> result = nullptr;
		throw_if_failed(D3D12CreateDevice(adapter, feature_level, IID_PPV_ARGS(&result)));
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

	void create_frame_contexts(uint32_t thread_count, ID3D12Device* device,
		std::array<FrameContext, MAX_CONCURRENT_GPU_FRAMES>& out)
	{
		for (uint32_t i = 0; i < MAX_CONCURRENT_GPU_FRAMES; i++)
		{
			auto& frame_ctx = out[i];

			device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frame_ctx.direct_queue_fence));
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frame_ctx.compute_queue_fence));
			frame_ctx.frame_fence_val = 0;

			frame_ctx.thread_data.reserve(thread_count);
			for (uint32_t j = 0; j < thread_count; j++)
			{
				frame_ctx.thread_data.push_back({
					.direct_queue_cmd_list_recycler =
						CommandListRecycler(device, D3D12_COMMAND_LIST_TYPE_DIRECT),
					.compute_queue_cmd_list_recycler =
						CommandListRecycler(device, D3D12_COMMAND_LIST_TYPE_COMPUTE)
					});
			}
		}
	}

	Context::Context(const Window& window, uint32_t thread_count)
		: m_thread_count(thread_count),
		m_factory(create_dxgi_factory()),
		m_adapter(create_adapter(m_factory.Get())),
		m_device(create_device(m_adapter.Get(), D3D_FEATURE_LEVEL_12_2)),
		m_direct_queue(create_command_queue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT)),
		m_compute_queue(create_command_queue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE)),
		m_copy_queue(create_command_queue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COPY)),
		m_swapchain(m_factory.Get(), m_direct_queue.Get(), window.get_hwnd(), m_device.Get()),
		m_frame_contexts()
	{
		create_frame_contexts(m_thread_count, m_device.Get(), m_frame_contexts);
	}

	Context::~Context()
	{
		await_gpu_idle();
	}

	FrameContext* Context::start_frame()
	{
		uint64_t frame_number = m_total_frames.load();

		auto frame = &m_frame_contexts[frame_number % MAX_CONCURRENT_GPU_FRAMES];
		wait_on_frame_context(frame);
		start_frame_context(frame);
		check_for_swapchain_resize();

		return frame;
	}

	void Context::end_frame(FrameContext* frame_context)
	{
		auto val = ++frame_context->frame_fence_val;
		m_direct_queue->Signal(frame_context->direct_queue_fence.Get(), val);
		m_compute_queue->Signal(frame_context->compute_queue_fence.Get(), val);
		m_total_frames.fetch_add(1);
	}

	void Context::await_gpu_idle()
	{
		ComPtr<ID3D12Fence1> fence = nullptr;
		m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		uint64_t target_value = 1;
		m_direct_queue->Signal(fence.Get(), target_value);
		wait_for_fence(fence.Get(), target_value++, INFINITE);
		m_compute_queue->Signal(fence.Get(), target_value);
		wait_for_fence(fence.Get(), target_value++, INFINITE);
		m_copy_queue->Signal(fence.Get(), target_value);
		wait_for_fence(fence.Get(), target_value, INFINITE);
	}

	IDXGISwapChain* Context::get_swapchain() const
	{
		return m_swapchain.get_dxgi_swapchain();
	}

	void Context::wait_on_frame_context(FrameContext* frame)
	{
		wait_for_fence(frame->direct_queue_fence.Get(), frame->frame_fence_val, INFINITE);
		wait_for_fence(frame->compute_queue_fence.Get(), frame->frame_fence_val, INFINITE);
	}

	void Context::start_frame_context(FrameContext* frame)
	{
		for (auto& td : frame->thread_data)
		{
			td.direct_queue_cmd_list_recycler.reset();
			td.compute_queue_cmd_list_recycler.reset();
		}
	}

	void Context::check_for_swapchain_resize()
	{
		HWND hwnd = {};
		auto dxgi_swapchain = m_swapchain.get_dxgi_swapchain();
		dxgi_swapchain->GetHwnd(&hwnd);
		uint32_t width, height;
		RECT rect = {};
		GetClientRect(hwnd, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
		DXGI_SWAP_CHAIN_DESC1 desc1 = {};
		dxgi_swapchain->GetDesc1(&desc1);
		if (width != desc1.Width || height != desc1.Height)
		{
			await_gpu_idle();
			m_swapchain.resize(width, height);
		}
	}
}
