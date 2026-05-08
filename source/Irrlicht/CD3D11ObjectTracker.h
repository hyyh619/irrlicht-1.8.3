#ifndef __IRR_D3D11_OBJECT_TRACKER_H_INCLUDED__
#define __IRR_D3D11_OBJECT_TRACKER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include "os.h"
#include "CD3D11Debug.h"

namespace irr
{
    namespace video
    {
        struct D3D11ObjectInfo
        {
            const void  *ptr;
            const char  *typeName;
            const char  *objectName;
            const char  *function;
            int         line;
        };

        inline void LogD3D11ObjectCreate(void *ptr, const char *typeName, const char *objectName, const char *func, int line)
        {
#if IRR_D3D11_TRACKING_ENABLED
            if (ptr)
            {
                char    msg[512];
                sprintf(msg, "[D3D11 CREATE] %s %s created at %s:%d ptr=0x%p", typeName, objectName, func, line, ptr);
                os::Printer::log(msg, ELL_INFORMATION);
            }
#endif
        }

        inline void LogD3D11ObjectAddRef(void *ptr, const char *typeName, const char *objectName, const char *func, int line)
        {
#if IRR_D3D11_TRACKING_ENABLED
            if (ptr)
            {
                char    msg[512];
                sprintf(msg, "[D3D11 ADDREF] %s %s AddRef at %s:%d ptr=0x%p", typeName, objectName, func, line, ptr);
                os::Printer::log(msg, ELL_INFORMATION);
            }
#endif
        }

        inline void LogD3D11ObjectRelease(void *ptr, const char *typeName, const char *objectName, const char *func, int line)
        {
#if IRR_D3D11_TRACKING_ENABLED
            if (ptr)
            {
                char    msg[512];
                sprintf(msg, "[D3D11 RELEASE] %s %s Release at %s:%d ptr=0x%p", typeName, objectName, func, line, ptr);
                os::Printer::log(msg, ELL_INFORMATION);
            }
#endif
        }

        inline void LogD3D11ObjectReleaseNoPtr(const char *typeName, const char *objectName, const char *func, int line)
        {
#if IRR_D3D11_TRACKING_ENABLED
            char    msg[512];
            sprintf(msg, "[D3D11 RELEASE] %s %s Release at %s:%d (NULL ptr)", typeName, objectName, func, line);
            os::Printer::log(msg, ELL_WARNING);
#endif
        }

        #define IRR_D3D11_CREATE(T, obj, name) \
    LogD3D11ObjectCreate((void*)(obj), #T, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_ADDREF(T, obj, name) \
    LogD3D11ObjectAddRef((void*)(obj), #T, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_RELEASE(T, obj, name)                                                      \
    do {                                                                                             \
        if (obj) LogD3D11ObjectRelease((void*)(obj), #T, name, __FUNCTION__, __LINE__); \
        else LogD3D11ObjectReleaseNoPtr(#T, name, __FUNCTION__, __LINE__);                           \
    } while (0)

        inline void LogD3D11BufferCreate(ID3D11Buffer *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11Buffer", name, func, line);
        }
        inline void LogD3D11BufferRelease(ID3D11Buffer *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11Buffer", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11Buffer", name, func, line);
        }
        inline void LogD3D11BufferAddRef(ID3D11Buffer *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectAddRef(obj, "ID3D11Buffer", name, func, line);
        }

        inline void LogD3D11Texture2DCreate(ID3D11Texture2D *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11Texture2D", name, func, line);
        }
        inline void LogD3D11Texture2DRelease(ID3D11Texture2D *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11Texture2D", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11Texture2D", name, func, line);
        }

        inline void LogD3D11SamplerStateCreate(ID3D11SamplerState *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11SamplerState", name, func, line);
        }
        inline void LogD3D11SamplerStateRelease(ID3D11SamplerState *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11SamplerState", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11SamplerState", name, func, line);
        }

        inline void LogD3D11BlendState1Create(ID3D11BlendState1 *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11BlendState1", name, func, line);
        }
        inline void LogD3D11BlendState1Release(ID3D11BlendState1 *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11BlendState1", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11BlendState1", name, func, line);
        }

        inline void LogD3D11DepthStencilViewCreate(ID3D11DepthStencilView *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11DepthStencilView", name, func, line);
        }
        inline void LogD3D11DepthStencilViewRelease(ID3D11DepthStencilView *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11DepthStencilView", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11DepthStencilView", name, func, line);
        }

        inline void LogD3D11DepthStencilStateCreate(ID3D11DepthStencilState *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11DepthStencilState", name, func, line);
        }
        inline void LogD3D11DepthStencilStateRelease(ID3D11DepthStencilState *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11DepthStencilState", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11DepthStencilState", name, func, line);
        }

        inline void LogD3D11RasterizerState1Create(ID3D11RasterizerState1 *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11RasterizerState1", name, func, line);
        }
        inline void LogD3D11RasterizerState1Release(ID3D11RasterizerState1 *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11RasterizerState1", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11RasterizerState1", name, func, line);
        }

        inline void LogD3D11RenderTargetViewCreate(ID3D11RenderTargetView *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11RenderTargetView", name, func, line);
        }
        inline void LogD3D11RenderTargetViewRelease(ID3D11RenderTargetView *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11RenderTargetView", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11RenderTargetView", name, func, line);
        }

        inline void LogD3D11ShaderResourceViewCreate(ID3D11ShaderResourceView *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11ShaderResourceView", name, func, line);
        }
        inline void LogD3D11ShaderResourceViewRelease(ID3D11ShaderResourceView *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11ShaderResourceView", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11ShaderResourceView", name, func, line);
        }

        inline void LogD3D11InputLayoutCreate(ID3D11InputLayout *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11InputLayout", name, func, line);
        }
        inline void LogD3D11InputLayoutRelease(ID3D11InputLayout *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11InputLayout", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11InputLayout", name, func, line);
        }

        inline void LogD3D11VertexShaderCreate(ID3D11VertexShader *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11VertexShader", name, func, line);
        }
        inline void LogD3D11VertexShaderRelease(ID3D11VertexShader *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11VertexShader", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11VertexShader", name, func, line);
        }

        inline void LogD3D11PixelShaderCreate(ID3D11PixelShader *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11PixelShader", name, func, line);
        }
        inline void LogD3D11PixelShaderRelease(ID3D11PixelShader *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11PixelShader", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11PixelShader", name, func, line);
        }

        inline void LogD3D11HullShaderCreate(ID3D11HullShader *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11HullShader", name, func, line);
        }
        inline void LogD3D11HullShaderRelease(ID3D11HullShader *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11HullShader", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11HullShader", name, func, line);
        }

        inline void LogD3D11DomainShaderCreate(ID3D11DomainShader *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11DomainShader", name, func, line);
        }
        inline void LogD3D11DomainShaderRelease(ID3D11DomainShader *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11DomainShader", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11DomainShader", name, func, line);
        }

        inline void LogD3D11GeometryShaderCreate(ID3D11GeometryShader *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11GeometryShader", name, func, line);
        }
        inline void LogD3D11GeometryShaderRelease(ID3D11GeometryShader *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease(obj, "ID3D11GeometryShader", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11GeometryShader", name, func, line);
        }

        inline void LogD3D11ComputeShaderCreate(ID3D11ComputeShader *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate(obj, "ID3D11ComputeShader", name, func, line);
        }
        inline void LogD3D11ComputeShaderRelease(ID3D11ComputeShader *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease((void*)obj, "ID3D11ComputeShader", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11ComputeShader", name, func, line);
        }

        inline void LogD3D11SwapChainCreate(IDXGISwapChain *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate((void*)obj, "IDXGISwapChain", name, func, line);
        }
        inline void LogD3D11SwapChainRelease(IDXGISwapChain *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease((void*)obj, "IDXGISwapChain", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("IDXGISwapChain", name, func, line);
        }

        inline void LogD3D11DeviceCreate(ID3D11Device *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate((void*)obj, "ID3D11Device", name, func, line);
        }
        inline void LogD3D11DeviceRelease(ID3D11Device *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease((void*)obj, "ID3D11Device", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11Device", name, func, line);
        }

        inline void LogD3D11DeviceContextCreate(ID3D11DeviceContext *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate((void*)obj, "ID3D11DeviceContext", name, func, line);
        }
        inline void LogD3D11DeviceContextRelease(ID3D11DeviceContext *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease((void*)obj, "ID3D11DeviceContext", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11DeviceContext", name, func, line);
        }

        inline void LogD3D11Device1Create(ID3D11Device1 *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate((void*)obj, "ID3D11Device1", name, func, line);
        }
        inline void LogD3D11Device1Release(ID3D11Device1 *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease((void*)obj, "ID3D11Device1", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11Device1", name, func, line);
        }

        inline void LogD3D11DebugCreate(ID3D11Debug *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectCreate((void*)obj, "ID3D11Debug", name, func, line);
        }
        inline void LogD3D11DebugRelease(ID3D11Debug *obj, const char *name, const char *func, int line)
        {
            if (obj)
                LogD3D11ObjectRelease((void*)obj, "ID3D11Debug", name, func, line);
            else
                LogD3D11ObjectReleaseNoPtr("ID3D11Debug", name, func, line);
        }

        inline void LogD3D11DeviceAddRef(ID3D11Device *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectAddRef((void*)obj, "ID3D11Device", name, func, line);
        }

        inline void LogD3D11DeviceContextAddRef(ID3D11DeviceContext *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectAddRef((void*)obj, "ID3D11DeviceContext", name, func, line);
        }

        inline void LogD3D11Device1AddRef(ID3D11Device1 *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectAddRef((void*)obj, "ID3D11Device1", name, func, line);
        }

        inline void LogD3D11DebugAddRef(ID3D11Debug *obj, const char *name, const char *func, int line)
        {
            LogD3D11ObjectAddRef((void*)obj, "ID3D11Debug", name, func, line);
        }

        #define IRR_D3D11_BUFFER_CREATE(obj, name)  LogD3D11BufferCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_BUFFER_RELEASE(obj, name) LogD3D11BufferRelease(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_BUFFER_ADDREF(obj, name)  LogD3D11BufferAddRef(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_TEXTURE2D_CREATE(obj, name)   LogD3D11Texture2DCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_TEXTURE2D_RELEASE(obj, name)  LogD3D11Texture2DRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_SAMPLER_CREATE(obj, name)     LogD3D11SamplerStateCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_SAMPLER_RELEASE(obj, name)    LogD3D11SamplerStateRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_BLEND_CREATE(obj, name)   LogD3D11BlendState1Create(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_BLEND_RELEASE(obj, name)  LogD3D11BlendState1Release(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_DSV_CREATE(obj, name)     LogD3D11DepthStencilViewCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DSV_RELEASE(obj, name)    LogD3D11DepthStencilViewRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_DSS_CREATE(obj, name)     LogD3D11DepthStencilStateCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DSS_RELEASE(obj, name)    LogD3D11DepthStencilStateRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_RS_CREATE(obj, name)  LogD3D11RasterizerState1Create(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_RS_RELEASE(obj, name) LogD3D11RasterizerState1Release(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_RTV_CREATE(obj, name)     LogD3D11RenderTargetViewCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_RTV_RELEASE(obj, name)    LogD3D11RenderTargetViewRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_SRV_CREATE(obj, name)     LogD3D11ShaderResourceViewCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_SRV_RELEASE(obj, name)    LogD3D11ShaderResourceViewRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_IL_CREATE(obj, name)  LogD3D11InputLayoutCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_IL_RELEASE(obj, name) LogD3D11InputLayoutRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_VS_CREATE(obj, name)  LogD3D11VertexShaderCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_VS_RELEASE(obj, name) LogD3D11VertexShaderRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_PS_CREATE(obj, name)  LogD3D11PixelShaderCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_PS_RELEASE(obj, name) LogD3D11PixelShaderRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_HS_CREATE(obj, name)  LogD3D11HullShaderCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_HS_RELEASE(obj, name) LogD3D11HullShaderRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_DS_CREATE(obj, name)  LogD3D11DomainShaderCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DS_RELEASE(obj, name) LogD3D11DomainShaderRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_GS_CREATE(obj, name)  LogD3D11GeometryShaderCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_GS_RELEASE(obj, name) LogD3D11GeometryShaderRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_CS_CREATE(obj, name)  LogD3D11ComputeShaderCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_CS_RELEASE(obj, name) LogD3D11ComputeShaderRelease(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_SWAPCHAIN_CREATE(obj, name)   LogD3D11SwapChainCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_SWAPCHAIN_RELEASE(obj, name)  LogD3D11SwapChainRelease(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DEVICE_CREATE(obj, name)  LogD3D11DeviceCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DEVICE_RELEASE(obj, name) LogD3D11DeviceRelease(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DEVICE_ADDREF(obj, name) LogD3D11DeviceAddRef(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_DEVICE_CONTEXT_CREATE(obj, name)  LogD3D11DeviceContextCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DEVICE_CONTEXT_RELEASE(obj, name) LogD3D11DeviceContextRelease(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DEVICE_CONTEXT_ADDREF(obj, name) LogD3D11DeviceContextAddRef(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_DEVICE1_CREATE(obj, name)     LogD3D11Device1Create(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DEVICE1_RELEASE(obj, name)    LogD3D11Device1Release(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DEVICE1_ADDREF(obj, name)    LogD3D11Device1AddRef(obj, name, __FUNCTION__, __LINE__)

        #define IRR_D3D11_DEBUG_CREATE(obj, name)   LogD3D11DebugCreate(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DEBUG_RELEASE(obj, name)  LogD3D11DebugRelease(obj, name, __FUNCTION__, __LINE__)
        #define IRR_D3D11_DEBUG_ADDREF(obj, name)  LogD3D11DebugAddRef(obj, name, __FUNCTION__, __LINE__)
    }
}
#endif
#endif