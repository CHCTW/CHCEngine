#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>
#include <stdexcept>
#include <string>
using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
inline std::string HrToString(HRESULT hr) {
  char s_str[64] = {};
  sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
  return std::string(s_str);
}
class HrException : public std::runtime_error {
 public:
  HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
  HRESULT Error() const { return m_hr; }

 private:
  const HRESULT m_hr;
};
inline void ThrowIfFailed(HRESULT hr) {
  if (FAILED(hr)) {
    throw HrException(hr);
  }
}
inline void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter,
                        bool requestHighPerformanceAdapter) {
  *ppAdapter = nullptr;

  ComPtr<IDXGIAdapter1> adapter;

  ComPtr<IDXGIFactory6> factory6;
  if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
    for (UINT adapterIndex = 0;
         DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
                                     adapterIndex,
                                     requestHighPerformanceAdapter == true
                                         ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                         : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                                     IID_PPV_ARGS(&adapter));
         ++adapterIndex) {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Don't select the Basic Render Driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create
      // the actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1,
                                      _uuidof(ID3D12Device), nullptr))) {
        break;
      }
    }
  } else {
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND !=
                                pFactory->EnumAdapters1(adapterIndex, &adapter);
         ++adapterIndex) {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Don't select the Basic Render Driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create
      // the actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1,
                                      _uuidof(ID3D12Device), nullptr))) {
        break;
      }
    }
  }

  *ppAdapter = adapter.Detach();
}
}  // namespace Renderer
}  // namespace CHCEngine