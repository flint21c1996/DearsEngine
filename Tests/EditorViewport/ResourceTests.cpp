#include "RenderTargetPool.h"
#include "GBuffer.h"
#include <iostream>

using Microsoft::WRL::ComPtr;
int main()
{
    ComPtr<ID3D11Device> device; ComPtr<ID3D11DeviceContext> context;
    const D3D_FEATURE_LEVEL requested[] = {D3D_FEATURE_LEVEL_11_0};
    D3D_FEATURE_LEVEL level;
    const HRESULT created = D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,requested,1,
        D3D11_SDK_VERSION,device.GetAddressOf(),&level,context.GetAddressOf());
    if (FAILED(created)) { std::cout << "WARP_UNAVAILABLE HRESULT=" << std::hex << created << '\n'; return 2; }
    RenderTargetPool pool; pool.Initialize(device.Get(),640,480);
    auto relative=pool.Create({.name="relative"});
    auto fixed=pool.Create({.name="fixed",.width=128,.height=64,.withDepth=false});
    if (!relative.IsValid() || !fixed.IsValid()) return 1;
    const auto oldResource=pool.Get(relative)->shaderResourceView;
    if (pool.Resize(relative,0,0) || pool.Get(relative)->width!=640) return 1;
    if (pool.Resize(relative,D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION+1,1))
    { std::cout << "FAIL resize accepted an extent beyond the D3D11 texture limit\n"; return 1; }
    if (pool.Get(relative)->shaderResourceView.Get()!=oldResource.Get())
    { std::cout << "FAIL failed resize replaced the held surface\n"; return 1; }
    if (!pool.ResizeDefaultTargets(2560,1440)) return 1;
    if (pool.Find("relative")!=relative || pool.Get(relative)->width!=2560 || pool.Get(relative)->height!=1440) return 1;
    if (pool.Get(fixed)->width!=128 || pool.Get(fixed)->height!=64) return 1;
    ComPtr<ID3D11Resource> oldTexture; oldResource->GetResource(oldTexture.GetAddressOf());
    ComPtr<ID3D11Texture2D> oldColor; if (FAILED(oldTexture.As(&oldColor))) return 1;
    D3D11_TEXTURE2D_DESC desc{}; oldColor->GetDesc(&desc);
    if (desc.Width!=640 || desc.Height!=480) return 1;
    std::cout << "PASS resize preserves handles and held prior SRV; fixed targets stay fixed\n";
    GBuffer gbuffer;
    if (!gbuffer.Initialize(device.Get(),context.Get(),640,480) || !gbuffer.Resize(2560,1440)) return 1;
    if (gbuffer.GetWidth()!=pool.Get(relative)->width || gbuffer.GetHeight()!=pool.Get(relative)->height) return 1;
    if (pool.ResizeDefaultTargets(0,0) || pool.Get(relative)->width!=2560) return 1;
    if (!pool.ResizeDefaultTargets(640,480) || !gbuffer.Resize(640,480)) return 1;
    std::cout << "PASS WARP FHD/QHD-compatible target and GBuffer resize with zero rejection\n";
    if (gbuffer.Resize(D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION+1,1) ||
        gbuffer.Resize(D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION+1,1) || !gbuffer.Resize(640,480)) return 1;
    std::cout << "PASS oversized extents are rejected and resources remain usable\n";
    std::cout << "RESULT 3 passed, 0 failed (software D3D11, not native hardware observation)\n";
    return 0;
}
