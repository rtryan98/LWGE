#if LWGE_BUILD_D3D12
#include "LWGE-RenderDriver/D3D12/D3D12RenderDriver.hpp"
#include "LWGE-RenderDriver/Util.hpp"

extern "C"
{
    __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 608;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

namespace lwge::rd::d3d12
{
    void throw_if_failed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw std::exception();
        }
    }

    DWORD wait_for_fence(ID3D12Fence1* fence, uint64_t target_value, uint32_t timeout)
    {
        if (fence->GetCompletedValue() < target_value)
        {
            HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
            throw_if_failed(fence->SetEventOnCompletion(target_value, event));
            if (!event)
            {
                throw std::exception();
            }
            return WaitForSingleObject(event, timeout);
        }
        return WAIT_FAILED;
    }

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

    D3D12RenderDriver::D3D12RenderDriver(const RenderDriverDesc& desc)
        : RenderDriver(desc),
        m_factory(create_dxgi_factory()),
        m_adapter(create_adapter(m_factory.Get())),
        m_device(create_device(m_adapter.Get(), D3D_FEATURE_LEVEL_12_2)),
        m_direct_queue(create_command_queue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT)),
        m_compute_queue(create_command_queue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE)),
        m_copy_queue(create_command_queue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COPY))
    {
        DXGI_ADAPTER_DESC adapter_desc = {};
        m_adapter->GetDesc(&adapter_desc);
        m_vendor = get_vendor_from_pci_id(adapter_desc.VendorId);
    }

    D3D12RenderDriver::~D3D12RenderDriver()
    {
        gpu_wait_idle();
    }

    void D3D12RenderDriver::gpu_wait_idle()
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
}
#endif // LWGE_BUILD_D3D12
