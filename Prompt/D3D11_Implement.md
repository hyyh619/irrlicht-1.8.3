# 1

对照source\Irrlicht\CD3D9*.cpp, source\Irrlicht\CD3D9*.h的代码，补全CD3D11*.cpp和CD3D11*.h的实现

# 2
1. 仿照examples里面的CONFORM_TEST代码，增加一个CONFORM_TEST_DX11的代码，使用driverType是EDT_DIRECT3D11
2. 并重写build_debug_x64_conform.bat脚本，使其可以同时支持CONFORM_TEST和CONFORM_TEST_DX11
3. 重命名CONFORM_TEST为CONFORM_TEST_DX9

# 3
1. 为examples 01~10 vc工程增加Debug_dx11_conform/x64的配置
2. Debug_dx11_conform/x64的配置从Debug/x64拷贝过来
3. Debug_dx11_conform需要定义宏CONFORM_TEST_DX11=1来开启dx11 conform test.

# 4
CD3D11HLSLMaterialRenderer.cpp和CD3D11ShaderMaterialRenderer.cpp在编译shader出错时，打印shader name.

# 5
使用D3DCompileFromFile(utf8ToUtf16(tmp).c_str(), 0, 0, "main", "vs_5_0", 0, 0, &code, &errors)编译D3D11_NORMAL_MAP_VSH时。
获得如下Error
Vertex shader compilation failed:
Shader name (first 64 chars):
;Irrlicht Engine D3D11 render path normal map vertex shader
; c0
C:\Development\Graphics\irrlicht-1.8.3\examples\01.HelloWorld\Shader@0x00007FF93DA17DA0(1,2-9): error X3000: unrecognized identifier 'Irrlicht'
看起来似乎是HLSL定义的comment使用了;开头的行不被认为是注释行。
修复上述问题

# 6
CD3D11Texture::createTexture下面部分的代码请参照CD3D9Texture::createTexture的实现
                case ETCF_OPTIMIZED_FOR_QUALITY:
                    break;

继续实现下列函数
        bool CD3D11Texture::createMipMaps(u32 level)
        {
            return false;
        }


        void CD3D11Texture::copy16BitMipMap(char *src, char *tgt,
                                            s32 width, s32 height, s32 pitchsrc, s32 pitchtgt) const
        {}


        void CD3D11Texture::copy32BitMipMap(char *src, char *tgt,
                                            s32 width, s32 height, s32 pitchsrc, s32 pitchtgt) const
        {}

# 7

修复下面的问题，copyTexture返回下面的错误
Could not map DIRECT3D11 Texture.

调用栈如下。
>	Irrlicht.dll!irr::video::CD2D11Texture::copyTexture(irr::video::IImage * image) 行 359	C++
 	Irrlicht.dll!irr::video::CD2D11Texture::CD3D11Texture(irr::video::IImage * image, irr::video::CD3D11Driver * driver, unsigned int flags, const irr::core::string<char,irr::core::irrAllocator<char>> & name, void * mipmapData) 行 60	C++

# 8
添加“desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;” 后，
“HRESULT    hr = Device->CreateTexture2D(&desc, 0, &Texture);”返回下列错误，请修复
0x00007FFA3379003C 处(位于 01.HelloWorld.exe 中)引发的异常: Microsoft C++ 异常: _com_error，位于内存位置 0x000000352A9CF338 处。
Could not create DIRECT3D11 Texture.

# 9
请修复执行context->UpdateSubresource(Texture, 0, &destBox, data, pitch, 0);时遇到的错误，错误如下
0x00007FFA2266A1F4 (prl_umdd10.dll)处(位于 01.HelloWorld.exe 中)引发的异常: 0xC0000005: 读取位置 0x0000023A8033E840 时发生访问冲突。

destBox参数如下
-		destBox	{left=0 top=0 front=0 ...}	D3D11_BOX
		left	0	unsigned int
		top	0	unsigned int
		front	0	unsigned int
		right	512	unsigned int
		bottom	256	unsigned int
		back	1	unsigned int

加载的data是从image获取，image的数据如下
-		image	0x0000023af7f5dda0 {Data=0x0000023a802bf040  <字符串中的字符无效。> Size={Width=308 Height=193 } BytesPerPixel=...}	irr::video::IImage * {irr::video::CImage}
-		[irr::video::CImage]	{Data=0x0000023a802bf040  <字符串中的字符无效。> Size={Width=308 Height=193 } BytesPerPixel=2 ...}	irr::video::CImage
+		irr::video::IImage	{...}	irr::video::IImage
+		irr::IReferenceCounted	{DebugName=0x00007ff93afbac90 "CImage" ReferenceCounter=1 }	irr::IReferenceCounted
+		Data	0x0000023a802bf040  <字符串中的字符无效。>	unsigned char *
-		Size	{Width=308 Height=193 }	irr::core::dimension2d<unsigned int>
		Width	308	unsigned int
		Height	193	unsigned int
		BytesPerPixel	2	unsigned int
		Pitch	616	unsigned int
		Format	ECF_A1R5G5B5 (0)	irr::video::ECOLOR_FORMAT
		DeleteMemory	true	bool

# 10
bool CD3D11Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const没有检查EVDF_TEXTURE_NPOT，请根据D3D11的Spec查看是否支持NPOT

# 11
仿照CD3D9Driver::initDriver代码
            // print device information
            D3DADAPTER_IDENTIFIER9 dai;
            if (!FAILED(pID3D->GetAdapterIdentifier(Params.DisplayAdapter, 0, &dai)))
            {
                char tmp[512];

                s32 Product    = HIWORD(dai.DriverVersion.HighPart);
                s32 Version    = LOWORD(dai.DriverVersion.HighPart);
                s32 SubVersion = HIWORD(dai.DriverVersion.LowPart);
                s32 Build      = LOWORD(dai.DriverVersion.LowPart);

                sprintf(tmp, "%s %s %d.%d.%d.%d", dai.Description, dai.Driver, Product, Version,
                    SubVersion, Build);
                os::Printer::log(tmp, ELL_INFORMATION);

                // Assign vendor name based on vendor id.
                VendorID = static_cast<u16>(dai.VendorId);

                switch (dai.VendorId)
                {
                    case 0x1002: VendorName = "ATI Technologies Inc."; break;

                    case 0x10DE: VendorName = "NVIDIA Corporation"; break;

                    case 0x102B: VendorName = "Matrox Electronic Systems Ltd."; break;

                    case 0x121A: VendorName = "3dfx Interactive Inc"; break;

                    case 0x5333: VendorName = "S3 Graphics Co., Ltd."; break;

                    case 0x8086: VendorName = "Intel Corporation"; break;

                    default: VendorName = "Unknown VendorId: "; VendorName += (u32)dai.VendorId; break;
                }
            }

            D3DDISPLAYMODE d3ddm;
            if (FAILED(pID3D->GetAdapterDisplayMode(Params.DisplayAdapter, &d3ddm)))
            {
                os::Printer::log("Error: Could not get Adapter Display mode.", ELL_ERROR);
                return false;
            }
实现CD3D11Driver::initDriver的相应功能。

根据DXGI_ADAPTER_DESC的定义，修复下面代码的问题
            DXGI_ADAPTER_DESC    desc;
            Adapter->GetDesc(&desc);

            char tmp[512];

            s32 Product    = HIWORD(desc.DriverVersion.HighPart);
            s32 Version    = LOWORD(desc.DriverVersion.HighPart);
            s32 SubVersion = HIWORD(desc.DriverVersion.LowPart);
            s32 Build      = LOWORD(desc.DriverVersion.LowPart);

参考d3d11
                case 0x05404c42: VendorName = "Parallel Desktop"; break;
为其他video driver增加新的vendor

# 12
1. 找到所有以CD3D11开头的class定义的非静态成员变量名字，
2. 逐个处理1找到的所有成员变量：
   A. 该成员变量名字如果不是以'm_'开头,就给成员变量的名字增加一个'm_'
   B. 更改所有该成员变量被引用的地方
   C. 注意不要更改成员函数名字，调用函数名字，数据结构名字

# 13
修复下列问题，
错误(活动)	E0020	未定义标识符 "VendorName"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h	242		
错误(活动)	E0020	未定义标识符 "DriverWasReset"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h	249		
警告	C26457	(void)不得用于忽略返回值，请使用 "std::ignore" 而不是 (es.48)。	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\include\irrAllocator.h	52		
警告	C6246	“tmp”的局部声明遮蔽了外部作用域中具有相同名称的声明。有关其他信息，请参见此前位于“90”行(“c:\development\graphics\irrlicht-1.8.3\source\irrlicht\cd3d11driver.cpp”中)的声明。	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp	140		
警告	C26461	指针参数 depth (针对函数 irr::video::CD3D11Driver::removeDepthSurface)可被标记为 const 指针(con.3)。	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp	1017		
警告	C26814	可在编译时计算常量变量“pureSoftware”。请考虑使用 constexpr (con.5)。	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp	1212		
警告	C26461	指针参数 p (针对函数 irr::video::CNullDriver::nml32)可被标记为 const 指针(con.3)。	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.h	709		
警告	C26461	指针参数 p (针对函数 irr::video::CNullDriver::nml16)可被标记为 const 指针(con.3)。	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.h	727		
错误	C2065	“VendorName”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h	242		
错误	C2065	“DriverWasReset”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h	249		
错误	C2065	“pID3DDeviceContext”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp	706		
错误	C2065	“VendorName”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h	242		
错误	C2065	“DriverWasReset”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h	249		
错误	C2065	“VendorName”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h	242		
错误	C2065	“DriverWasReset”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h	249		
错误	C2039	"pID3DDevice": 不是 "irr::video::CD3D11Driver" 的成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	30		
错误	C2039	"pID3DDevice": 不是 "irr::video::CD3D11Driver" 的成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	52		
错误	C2039	"pID3DDeviceContext": 不是 "irr::video::CD3D11Driver" 的成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	147		
错误	C2039	"pID3DDeviceContext": 不是 "irr::video::CD3D11Driver" 的成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	152		
错误	C2039	"pID3DDeviceContext": 不是 "irr::video::CD3D11Driver" 的成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	352		
错误	C2039	"pID3DDeviceContext": 不是 "irr::video::CD3D11Driver" 的成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	373		
错误	C2039	"pID3DDeviceContext": 不是 "irr::video::CD3D11Driver" 的成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	378		
错误	C2039	"pID3DDeviceContext": 不是 "irr::video::CD3D11Driver" 的成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	385		
错误	C2065	“CallBack”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11NormalMapRenderer.cpp	119		
错误	C2065	“CallBack”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11NormalMapRenderer.cpp	139		
错误	C2065	“CallBack”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11NormalMapRenderer.cpp	140		

# 14
1. 找到所有以CD3D9开头的class定义的非静态成员变量名字，
2. 逐个处理1找到的所有成员变量：
   A. 该成员变量名字如果不是以'm_'开头,就给成员变量的名字增加一个'm_'
   B. 更改所有该成员变量被引用的地方
   C. 注意不要更改成员函数名字，调用函数名字，数据结构名字

# 15
修复下列问题，
错误(活动)	E1696	无法打开 源 文件 "CD3D9m_MaterialRenderer.h"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	13		
错误(活动)	E1696	无法打开 源 文件 "CD3D9Shaderm_MaterialRenderer.h"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	14		
错误(活动)	E1696	无法打开 源 文件 "CD3D9HLSLm_MaterialRenderer.h"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	17		
错误(活动)	E1696	无法打开 源 文件 "CD3D9Cgm_MaterialRenderer.h"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	18		

修复下列问题，
错误(活动)	E0020	未定义标识符 "deletem_MaterialRenders"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	84		
错误(活动)	E0020	未定义标识符 "addm_MaterialRenderer"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	125		
错误(活动)	E0135	命名空间 "irr::video" 没有成员 "Im_MaterialRenderer"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	148		
错误(活动)	E0020	未定义标识符 "renderer"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	148		
错误(活动)	E0020	未定义标识符 "m_MaterialRenderers"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	151		
错误(活动)	E0020	未定义标识符 "D3Dm_ColorFormat"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	506		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "m_Caps3"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	618		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "Devm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	663		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "Texturem_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	666		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "Texturem_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	707		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "Texturem_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	710		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "PrimitiveMiscm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	713		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "PrimitiveMiscm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	719		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "PrimitiveMiscm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	722		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "Rasterm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	728		
错误(活动)	E0020	未定义标识符 "CurrentTexture"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	792		
错误(活动)	E0135	类 "irr::video::CD3D9Texture" 没有成员 "DepthSurface"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	927		
错误(活动)	E0135	类 "irr::video::CD3D9Texture" 没有成员 "DepthSurface"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	1055		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	1552		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialTypeParam"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	1558		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	1562		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	1562		
错误(活动)	E0020	未定义标识符 "LastMaterial"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2169		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2174		
错误(活动)	E0020	未定义标识符 "m_MaterialRenderers"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2175		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2180		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2180		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2181		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2186		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2186		
错误(活动)	E0020	未定义标识符 "m_MaterialRenderers"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2186		
错误(活动)	E0135	类 "irr::video::SMaterial" 没有成员 "m_MaterialType"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2187		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "TextureAddressm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2205		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "TextureAddressm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2210		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "TextureAddressm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2214		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "TextureAddressm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2218		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "TextureAddressm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2226		


修复下列问题，
错误(活动)	E0020	未定义标识符 "m_m_CurrentTexture"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	54		
错误(活动)	E0020	未定义标识符 "deleteMaterialRenderers"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	84		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "Texturem_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	666		
错误(活动)	E0147	声明与 "void irr::video::CD3D9Driver::setBasicRenderStates(const irr::video::SMaterial &material, const irr::video::SMaterial &lastMaterial, bool resetAllRenderstates)" (已声明 所在行数:253，所属文件:"C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.h") 不兼容	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2238		
错误(活动)	E0020	未定义标识符 "Sm_Material"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2238		
错误(活动)	E0020	未定义标识符 "Sm_Material"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2238		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "TextureFilterm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2546		
错误(活动)	E0135	类 "_D3DCAPS9" 没有成员 "TextureFilterm_Caps"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2548		
错误(活动)	E0304	没有与参数列表匹配的 重载函数 "irr::core::min_" 实例	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2553		
错误(活动)	E0135	类 "irr::video::CD3D9Driver" 没有成员 "enablem_Material2D"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2696		
错误(活动)	E0020	未定义标识符 "m_CurrentRenderMode"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2699		
错误(活动)	E0020	未定义标识符 "ERM_NONE"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2699		
错误(活动)	E0135	类 "irr::video::CNullDriver" 没有成员 "enablem_Material2D"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	2701		
错误(活动)	E0135	类 "irr::video::CD3D9Texture" 没有成员 "DepthSurface"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	3114		
错误(活动)	E0135	类 "irr::video::CD3D9Driver" 没有成员 "getm_D3DColorFormat"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	3538		
错误(活动)	E1670	非成员函数上不允许使用类型限定符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	3538		
错误(活动)	E0020	未定义标识符 "m_D3DColorFormat"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	3540		
错误(活动)	E0135	类 "irr::video::CD3D9Driver" 没有成员 "getD3DFormatFromm_ColorFormat"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	3588		
错误(活动)	E1670	非成员函数上不允许使用类型限定符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	3588		
错误(活动)	E0135	类 "irr::video::CD3D9Texture" 没有成员 "DepthSurface"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp	3741		
错误	C2065	“VendorName”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.h	309		
错误	C2065	“DriverWasReset”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.h	318		
错误	C2614	“irr::video::CD3D9MaterialRenderer”: 非法的成员初始化:“pID3DDevice”不是基或成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	61		
错误	C2614	“irr::video::CD3D9MaterialRenderer”: 非法的成员初始化:“Driver”不是基或成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	61		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	115		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	119		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	120		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	148		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	152		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	153		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	154		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	157		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	164		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	168		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	172		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	177		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	261		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	263		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	264		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	266		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	287		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	290		
错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	291		

# 16
参考CD3D9Driver::beginScene实现，修复CD3D11Driver::beginScene的问题，
目前我已经看到两个问题
1. 未调用CNullDriver::beginScene
2. backBuffer, zBuffer的判断错误

# 17
d3d11最大可以支持8个纹理,其它video driver支持4个,改一下下面的定义
#define _IRR_MATERIAL_MAX_TEXTURES_ 4

# 18
根据CD3D9Driver::setVertexShader的代码，实现CD3D11Driver::setVertexShader

# 19
1. 参考CD3D9Driver的代码，实现CD3D11Driver 的硬件缓冲区管理，包括drawHardwareBuffer 和 updateVertexHardwareBuffer的实现像 D3D9 那样设置 vertex shader/FVF。
2. D3D11 需要通过 Input Layout 来定义顶点格式，实现 input layout 管理机制。你需要实现：
   A. 为每种 E_VERTEX_TYPE 创建/缓存 ID3D11InputLayout
   B. 在 setVertexShader 中调用 IASetInputLayout
   C. 可能需要内置的 simple vertex shader 来处理基本渲染
我没有内置 vertex shader 代码（如 EVT_STANDARD、EVT_2TCOORDS、EVT_TANGENTS 对应的 shader），帮我实现完整的版本。

# 20
参照CD3D9Driver::draw2D3DVertexPrimitiveList的代码，实现CD3D11Driver::draw2D3DVertexPrimitiveList

# 21
每次调用CD3D11Driver::draw2D3DVertexPrimitiveList都需要创建vertex buffer和index buffer。我们没必要每次都创建一个临时buffer，绘制完成后就立马释放。优化这部分代码。

# 22
在d3d11中设置的model/view/project矩阵没有被设置给渲染管线。
1. 请在draw前，通过model/view/project matrics计算出mvp matrix
2. mvp matrix作为constant传递给vertex shader
3. vertex shader的顶点计算增加mvp矩阵转换。

# 23
d3d11的渲染管线需要设置viewport/scissor, ID3D11DepthStencilState, ID3D11RasterizerState1和ID3D11BlendState1，请增加这些设置项。

# 24
d3d11的纹理采样的sampler，只有一个m_SamplerState，请做如下改动，
1. 增加Sampler类，能够保存不同的d3d11的sampler设置，以及创建的sampler
2. 初始化创建一个默认的sampler类对象供pixel shader采样使用。

# 25
为CD3D11Driver::createDefaultStates的所有FAILED判断增加Log输出
            if (FAILED(hr))
                return false;

# 26
d3d11在draw前调用CD3D11Driver::setMaterial设置m_CurrentTexture，因此我们需要在draw被执行前，为Pixel shader设置纹理。
请生成一个PS的纹理和采样器配置函数，该函数需要在 m_pID3DDeviceContext->PSSetShader调用后执行

# 27
d3d11的PIXEL_SHADER_STANDARD需要采样一个纹理的纹素，把该纹素作为PS的输出color

# 28
d3d9使用下面3个矩阵完成MVP转换
            // ! View transformation
            ETS_VIEW = 0,
            // ! World transformation
            ETS_WORLD,
            // ! Projection transformation
            ETS_PROJECTION,
分别对应设置
                case ETS_VIEW:
                    m_pID3DDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)((void*)mat.pointer()));
                    break;

                case ETS_WORLD:
                    m_pID3DDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)((void*)mat.pointer()));
                    break;

                case ETS_PROJECTION:
                    m_pID3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)((void*)mat.pointer()));
                    break;

d3d11只需要给VS一个MVP矩阵，完成顶点转换。获得了ETS_VIEW，ETS_WORLD和ETS_PROJECTION矩阵，该如何计算出VS使用的MVP矩阵。

# 29
CD3D11Texture::copyTexture在拷贝image数据到texture对应的resource中时，
1. 需要先检查image的format与m_ColorFormat是否匹配。
2. 如果不匹配需要使用m_ColorFormat创建一个临时的tmpImage
3. 使用image的copyToScaling把原始image的数据拷贝到tmpImage中
4. 使用tmpImage的data 上传到texture的resource中。

# 30
创建shader类
1. 该类表示d3d11的所有shader类型，包括vs/hs/ds/gs/ps/cs
2. 该类保存shader HLSL源代码
3. 如果是vs，还需要保存input layout信息以及创建的ID3D11InputLayout
4. HLSL经过D3DCompile编译后生成的ID3DBlob也需要保存
5. 需要保存创建的ID3D11VertexShader，ID3D11PixelShader，ID3D11DomainShader，ID3D11HullShader,ID3D11GeometryShader,ID3D11ComputeShader等对象
6. 该类的对象由CD3D11Driver负责创建，管理和销毁
7. 把下列shader源码的编译，创建都使用该shader类来管理

# 31
1. m_ShaderPool缓存了创建的shader对象，但是在最后CD3D11Driver对象销毁时，没有释放
2. 下面的成员变量定义数组的大小不要使用数字，根据当前shader type,创建一个shader type enum，根据enum的数量来创建数组
            ID3D11InputLayout               *m_InputLayout[3];
            ID3D11VertexShader              *m_BuiltInVertexShader[3];
            ID3D11PixelShader               *m_BuiltInPixelShader[3];

# 32
为每个ID3D11* 对象的创建，引用和销毁都打印一个log来跟踪其生存周期，以及检测内存泄漏，
打印的log要包括对象的ID3D11*的指针地址，以及在哪个函数被调用。
这些ID3D11* 对象例如：
ID3D11Buffer
ID3D11SamplerState
ID3D11BlendState1
ID3D11DepthStencilView
请包括所有的对象。

# 33
1. 检查CD3D11Driver::~CD3D11Driver 中 SwapChain 和 Device 的释放逻辑，确保没有memory leak
2. 确保 DepthStencilTexture 在 initDriver 失败回滚时也有 Release
3. 在CD3D11Driver释放结束时使用 ID3D11Debug::ReportLiveDeviceObjects 看到更完整的未释放对象列表，这个只对Debug代码有效
4. 检查DepthStencilTexture， SwapChain， ID3D11Device/Device1是否释放，如果没有，添加释放代码。

# 34
CD3D11ShaderMaterialRenderer创建了下列d3d11的对象，没有释放。请释放。
            ID3D11Device                *m_pID3DDevice;
            ID3D11DeviceContext         *m_pID3DDeviceContext;
            ID3D11VertexShader          *m_VertexShader;
            ID3D11VertexShader          *m_OldVertexShader;
            ID3D11PixelShader           *m_PixelShader;
            ID3D11InputLayout           *m_InputLayout;

# 35
CD3D11Driver::createMaterialRenderers创建了下列对象，但是只是到入CNullDriver的MaterialRenderers进行管理。
            new CD3D11MaterialRenderer(this, matType, "solid");
            new CD3D11MaterialRenderer(this, matType, "solid_lightmap");
            new CD3D11MaterialRenderer(this, matType, "solid_2_layer");
            new CD3D11MaterialRenderer(this, matType, "translucent");
            new CD3D11MaterialRenderer(this, matType, "translucent_2_layer");
            new CD3D11MaterialRenderer(this, matType, "translucent_add_color");
            new CD3D11MaterialRenderer(this, matType, "translucent_vertex_alpha");
            new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel");
            new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel_ref");
            new CD3D11MaterialRenderer(this, matType, "one_texture_blend");
            new CD3D11MaterialRenderer(this, matType, "lightmap_blend");
            new CD3D11MaterialRenderer(this, matType, "detail_map");
            new CD3D11MaterialRenderer(this, matType, "sphere_map");
            new CD3D11MaterialRenderer(this, matType, "reflection_2_layer");
            new CD3D11MaterialRenderer(this, matType, "transparent_reflection_2_layer");

            if (queryFeature(video::EVDF_PIXEL_SHADER_1_1) && queryFeature(video::EVDF_VERTEX_SHADER_1_1))
            {
                new CD3D11NormalMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
                new CD3D11ParallaxMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
            }
1. CD3D11MaterialRenderer创建时其ReferenceCounter就为1了
2. 调用CNullDriver::addMaterialRenderer加入到MaterialRenderers其ReferenceCounter为2
3. 但是在释放时只调用了CNullDriver::deleteMaterialRenders,因此其ReferenceCounter只会减到1，不会调用delete
4. 在CD3D11Driver::createMaterialRenderers创建MaterialRenderer对象时，请添加CD3D11Driver自己的管理对象，在CD3D11Driver释放时，先调用CNullDriver::deleteMaterialRenders，再调用CD3D11Driver自己的管理对象对MaterialRenderer对象进行释放。

# 36
为CD3D11NormalMapRenderer，CD3D11MaterialRenderer，CD3D11ShaderMaterialRenderer的构造函数和析构函数增加一个log打印
1. 该打印需要打印当前对象的指针
2. 该打印用于跟踪对象的创建和释放，只在Debug驱动时有效。

# 37
给所有引用和释放ID3D11Device1，ID3D11Device的地方加上一个log来跟踪其生存周期，以及检测内存泄漏，
打印的log要包括对象的ID3D11Device1, ID3D11Device的指针地址，以及在哪个函数被调用

请检查CD3D11Texture，CD3D11ShaderMaterialRenderer是否用到ID3D11Device1，ID3D11Device，有没有添加相应的AddRef和Release，以及对应的Log打印。

# 38
参考 CD3D9Driver::draw2DRectangle的实现，实现CD3D11Driver::draw2DRectangle。主要功能就是Draws a 2d rectangle with a gradient
1. 创建4个顶点，绘制两个三角形组成一个rectangle
2. 4个顶点分别赋予四个颜色SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown
3. Shader使用CShader对象管理

# 39
为CD3D11Driver::draw2DRectangle创建一组新的VS和PS，VS只需要输入pos,color，输出pos,color，PS只需要输入color，输出color。不要使用原来的video::EVT_STANDARD

# 40
1. CD3D11Driver::createRectangleShaders创建的CShader对象也要用m_ShaderPool来管理
2. CD3D11Driver::draw2DRectangle切换shader时，也要使用setShadersByType来设置
3. 为rectangle shaders增加一个新的shader类型EVT_2D_RECTANGLE
4. CShader增加一个成员变量记录E_VERTEX_TYPE
5. CD3D11Driver增加一个查询函数，根据输入的E_VERTEX_TYPE和E_D3D11_SHADER_TYPE在m_ShaderPool中查找对应的CShader对象

# 41 
Git commit: Implement CD3D11Driver::draw2DRectangle
1. 把CD3D11Driver::createRectangleShaders创建的shader也加入到m_BuiltInVertexShader和m_BuiltInPixelShader中统一管理
2. CD3D11Driver::draw2DRectangle设置shader时，要像CD3D11Driver::draw2D3DVertexPrimitiveList一样使用setShadersByType来切换，而不是直接调用下面代码
            CD3D11Shader    *vsShader   = getShaderByTypes(EVT_2D_RECTANGLE, EDST_VERTEX);
            CD3D11Shader    *psShader   = getShaderByTypes(EVT_2D_RECTANGLE, EDST_PIXEL);

            if (vsShader)
                m_pID3DDeviceContext->VSSetShader(vsShader->getVertexShader(), 0, 0);

            if (psShader)
                m_pID3DDeviceContext->PSSetShader(psShader->getPixelShader(), 0, 0);

            if (vsShader)
                m_pID3DDeviceContext->IASetInputLayout(vsShader->getInputLayout());

# 42 
Git commit: Created SRenderStateSet struct containing RasterizerState, DepthStencilState, BlendState by MiniMax-M2.7.
1. m_RasterizerState，m_DepthStencilState，m_BlendState只是为ERM_3D使用，我们需要为ERM_2D创建另外一组m_RasterizerState，m_DepthStencilState，m_BlendState
2. ERM_2D的m_RasterizerState，m_DepthStencilState，m_BlendState，要关闭depth/stencil/blend。
3. 多组m_RasterizerState，m_DepthStencilState，m_BlendState状态，需要创建一个数据结构统一管理
4. CD3D11Driver::setRenderStates调用时，根据E_RENDER_MODE来选择对应的states。

# 43
Git commit: Add mvp for 2d rectangle by MiniMax-M2.7.
CD3D11Driver::draw2DRectangle收到的pos是屏幕像素坐标，我们需要在vs里面经过mvp矩阵把其转换到NDC坐标中
1. 改动VERTEX_SHADER_RECTANGLE，支持MVP变换
2. 生成CD3D11Driver::draw2DRectangle转换屏幕像素坐标到NDC坐标的MVP矩阵
3. 执行CD3D11Driver::draw2DRectangle的draw前把MVP矩阵作为constant给到VS

# 44
Git commit: Implement draw2DImageBatch by MiniMax-M2.7.
根据CD3D9Driver::draw2DImageBatch的实现，实现CD3D11Driver::draw2DImageBatch

# 45
Git commit: Add alpha blend to render states for draw2DImageBatch by MiniMax-M2.7.
1. SRenderStateSet的BlendState需要增加一个，两个BlendState分别是开启alpha和不开启alpha的state
2. CD3D11Driver::setRenderStates根据alpha是否开启，来选择正确的blendstate

# 46
Git commit: Chang font's color from white to black which uses alpha blend in PS by MiniMax-M2.7.
分析一下下列d3d9调用的作用。
m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
m_pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
m_pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
m_pID3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
m_pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
m_pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
m_pID3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
m_pID3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
m_pID3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

根据下面的d3d9的计算公式改造PIXEL_SHADER_STANDARD，只是alpha混合
这些D3D9调用配置了典型的Alpha透明渲染：
Alpha混合设置
调用
ALPHABLENDENABLE = TRUE
SRCBLEND = SRCALPHA
DESTBLEND = INVSRCALPHA
混合公式: Final = Source × α + Dest × (1-α)（标准Alpha混合）
纹理颜色操作 (Stage 0)
COLOROP = MODULATE              // 操作：相乘
COLORARG1 = TEXTURE             // 参数1：纹理颜色
COLORARG2 = DIFFUSE             // 参数2：顶点漫反射色
结果: FinalColor = TextureColor × DiffuseColor
纹理Alpha操作 (Stage 0)
ALPHAOP = MODULATE              // 操作：相乘
ALPHAARG1 = TEXTURE             // 参数1：纹理Alpha
ALPHAARG2 = DIFFUSE             // 参数2：顶点漫反射Alpha
结果: FinalAlpha = TextureAlpha × DiffuseAlpha
总结
这是渲染带纹理的半透明物体（如UI元素、粒子效果）的标准配置。纹理颜色与顶点颜色调制产生最终颜色，Alpha值也通过相同方式计算，然后使用标准的SRCALPHA/INVSRCALPHA混合公式与背景混合。

# 46
Git commit: add draw dump for each draw call by MiniMax-M2.7.
增加一个dump每个draw call绘制的图像的接口，通过一个宏开关来控制开启。开启这个功能，每个draw绘制后的内容被dump到一张jpg图片里.

# 47
Git commit: Add draw dump for all draw calls by MiniMax-M2.7.
1. 为CD3D9Driver::draw*，CD3D11Driver::draw*, COpenGLDriver::draw*的所有draw绘制都加上dumpDrawCall
2. dumpDrawCall输出的文件名需要包含当前draw类型，例如draw2D3DVertexPrimitiveList，draw2DRectangle, draw2DImageBatch等等。

# 48
Git commit: Separate VS and PS creation and setting for PS material by MiniMax-M2.7.
1. CShader 增加一个E_MATERIAL_TYPE类型的成员变量
2. CShader创建时，如果是PS，需要设置E_MATERIAL_TYPE类型的成员变量，其他shader type设成EMT_SOLID
3. CShader增加setMaterialType和getMaterialType成员函数
4. CD3D11Driver::getShaderByTypes增加一个E_MATERIAL_TYPE类型的判断
5. CD3D11Driver::setShadersByType改成CD3D11Driver::setVSByType，只保留VS设置
6. 新增CD3D11Driver::setPSByType函数，把CD3D11Driver::setShadersByType函数内PS设置相关的代码放到CD3D11Driver::setPSByType函数，CD3D11Driver::setPSByType函数根据E_MATERIAL_TYPE和video::E_VERTEX_TYPE类型来选择对应的PS

优化下列三个Shader创建函数，改成统一的VS创建和PS创建,VS创建在CD3D11Driver::setVSByType初始化，PS创建在CD3D11Driver::setPSByType
createBuiltInVertexShader((E_VERTEX_TYPE)i);
createBuiltInPixelShader((E_VERTEX_TYPE)i);
createRectangleShaders();

# 49
Git commit: Create PS HLSL by materials by MiniMax-M2.7.
根据下面material的类型，生成对应的PS HLSL代码
            //! Standard solid material.
            /** Only first texture is used, which is supposed to be the
             * diffuse material. */
            EMT_SOLID = 0,

            //! Solid material with 2 texture layers.
            /** The second is blended onto the first using the alpha value
             * of the vertex colors. This material is currently not implemented in OpenGL.
             */
            EMT_SOLID_2_LAYER,

            //! Material type with standard lightmap technique
            /** There should be 2 textures: The first texture layer is a
             * diffuse map, the second is a light map. Dynamic light is
             * ignored. */
            EMT_LIGHTMAP,

            //! Material type with lightmap technique like EMT_LIGHTMAP.
            /** But lightmap and diffuse texture are added instead of modulated. */
            EMT_LIGHTMAP_ADD,

            //! Material type with standard lightmap technique
            /** There should be 2 textures: The first texture layer is a
             * diffuse map, the second is a light map. Dynamic light is
             * ignored. The texture colors are effectively multiplied by 2
             * for brightening. Like known in DirectX as D3DTOP_MODULATE2X. */
            EMT_LIGHTMAP_M2,

            //! Material type with standard lightmap technique
            /** There should be 2 textures: The first texture layer is a
             * diffuse map, the second is a light map. Dynamic light is
             * ignored. The texture colors are effectively multiplyied by 4
             * for brightening. Like known in DirectX as D3DTOP_MODULATE4X. */
            EMT_LIGHTMAP_M4,

            //! Like EMT_LIGHTMAP, but also supports dynamic lighting.
            EMT_LIGHTMAP_LIGHTING,

            //! Like EMT_LIGHTMAP_M2, but also supports dynamic lighting.
            EMT_LIGHTMAP_LIGHTING_M2,

            //! Like EMT_LIGHTMAP_4, but also supports dynamic lighting.
            EMT_LIGHTMAP_LIGHTING_M4,

            //! Detail mapped material.
            /** The first texture is diffuse color map, the second is added
             * to this and usually displayed with a bigger scale value so that
             * it adds more detail. The detail map is added to the diffuse map
             * using ADD_SIGNED, so that it is possible to add and substract
             * color from the diffuse map. For example a value of
             * (127,127,127) will not change the appearance of the diffuse map
             * at all. Often used for terrain rendering. */
            EMT_DETAIL_MAP,

            //! Look like a reflection of the environment around it.
            /** To make this possible, a texture called 'sphere map' is
             * used, which must be set as the first texture. */
            EMT_SPHERE_MAP,

            //! A reflecting material with an optional non reflecting texture layer.
            /** The reflection map should be set as first texture. */
            EMT_REFLECTION_2_LAYER,

            //! A transparent material.
            /** Only the first texture is used. The new color is calculated
             * by simply adding the source color and the dest color. This
             * means if for example a billboard using a texture with black
             * background and a red circle on it is drawn with this material,
             * the result is that only the red circle will be drawn a little
             * bit transparent, and everything which was black is 100%
             * transparent and not visible. This material type is useful for
             * particle effects. */
            EMT_TRANSPARENT_ADD_COLOR,

            //! Makes the material transparent based on the texture alpha channel.
            /** The final color is blended together from the destination
             * color and the texture color, using the alpha channel value as
             * blend factor. Only first texture is used. If you are using
             * this material with small textures, it is a good idea to load
             * the texture in 32 bit mode
             * (video::IVideoDriver::setTextureCreationFlag()). Also, an alpha
             * ref is used, which can be manipulated using
             * SMaterial::MaterialTypeParam. This value controls how sharp the
             * edges become when going from a transparent to a solid spot on
             * the texture. */
            EMT_TRANSPARENT_ALPHA_CHANNEL,

            //! Makes the material transparent based on the texture alpha channel.
            /** If the alpha channel value is greater than 127, a
             * pixel is written to the target, otherwise not. This
             * material does not use alpha blending and is a lot faster
             * than EMT_TRANSPARENT_ALPHA_CHANNEL. It is ideal for drawing
             * stuff like leafes of plants, because the borders are not
             * blurry but sharp. Only first texture is used. If you are
             * using this material with small textures and 3d object, it
             * is a good idea to load the texture in 32 bit mode
             * (video::IVideoDriver::setTextureCreationFlag()). */
            EMT_TRANSPARENT_ALPHA_CHANNEL_REF,

            //! Makes the material transparent based on the vertex alpha value.
            EMT_TRANSPARENT_VERTEX_ALPHA,

            //! A transparent reflecting material with an optional additional non reflecting texture layer.
            /** The reflection map should be set as first texture. The
             * transparency depends on the alpha value in the vertex colors. A
             * texture which will not reflect can be set as second texture.
             * Please note that this material type is currently not 100%
             * implemented in OpenGL. */
            EMT_TRANSPARENT_REFLECTION_2_LAYER,

            //! A solid normal map renderer.
            /** First texture is the color map, the second should be the
             * normal map. Note that you should use this material only when
             * drawing geometry consisting of vertices of type
             * S3DVertexTangents (EVT_TANGENTS). You can convert any mesh into
             * this format using IMeshManipulator::createMeshWithTangents()
             * (See SpecialFX2 Tutorial). This shader runs on vertex shader
             * 1.1 and pixel shader 1.1 capable hardware and falls back to a
             * fixed function lighted material if this hardware is not
             * available. Only two lights are supported by this shader, if
             * there are more, the nearest two are chosen. */
            EMT_NORMAL_MAP_SOLID,

            //! A transparent normal map renderer.
            /** First texture is the color map, the second should be the
             * normal map. Note that you should use this material only when
             * drawing geometry consisting of vertices of type
             * S3DVertexTangents (EVT_TANGENTS). You can convert any mesh into
             * this format using IMeshManipulator::createMeshWithTangents()
             * (See SpecialFX2 Tutorial). This shader runs on vertex shader
             * 1.1 and pixel shader 1.1 capable hardware and falls back to a
             * fixed function lighted material if this hardware is not
             * available. Only two lights are supported by this shader, if
             * there are more, the nearest two are chosen. */
            EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR,

            //! A transparent (based on the vertex alpha value) normal map renderer.
            /** First texture is the color map, the second should be the
             * normal map. Note that you should use this material only when
             * drawing geometry consisting of vertices of type
             * S3DVertexTangents (EVT_TANGENTS). You can convert any mesh into
             * this format using IMeshManipulator::createMeshWithTangents()
             * (See SpecialFX2 Tutorial). This shader runs on vertex shader
             * 1.1 and pixel shader 1.1 capable hardware and falls back to a
             * fixed function lighted material if this hardware is not
             * available.  Only two lights are supported by this shader, if
             * there are more, the nearest two are chosen. */
            EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA,

            //! Just like EMT_NORMAL_MAP_SOLID, but uses parallax mapping.
            /** Looks a lot more realistic. This only works when the
             * hardware supports at least vertex shader 1.1 and pixel shader
             * 1.4. First texture is the color map, the second should be the
             * normal map. The normal map texture should contain the height
             * value in the alpha component. The
             * IVideoDriver::makeNormalMapTexture() method writes this value
             * automatically when creating normal maps from a heightmap when
             * using a 32 bit texture. The height scale of the material
             * (affecting the bumpiness) is being controlled by the
             * SMaterial::MaterialTypeParam member. If set to zero, the
             * default value (0.02f) will be applied. Otherwise the value set
             * in SMaterial::MaterialTypeParam is taken. This value depends on
             * with which scale the texture is mapped on the material. Too
             * high or low values of MaterialTypeParam can result in strange
             * artifacts. */
            EMT_PARALLAX_MAP_SOLID,

            //! A material like EMT_PARALLAX_MAP_SOLID, but transparent.
            /** Using EMT_TRANSPARENT_ADD_COLOR as base material. */
            EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR,

            //! A material like EMT_PARALLAX_MAP_SOLID, but transparent.
            /** Using EMT_TRANSPARENT_VERTEX_ALPHA as base material. */
            EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA,

            //! BlendFunc = source * sourceFactor + dest * destFactor ( E_BLEND_FUNC )
            /** Using only first texture. Generic blending method. */
            EMT_ONETEXTURE_BLEND,

            //! This value is not used. It only forces this enumeration to compile to 32 bit.
            EMT_FORCE_32BIT = 0x7fffffff

# 50
Git commit: Create PS HLSL by materials by MiniMax-M2.7.
以前的代码是PS根据E_VERTEX_TYPE创建
                for (u32 i = 0; i < EVT_2D_RECTANGLE; ++i)
                {
                    createBuiltInPixelShader((E_VERTEX_TYPE)i);
                }
改动代码如下：
1. CD3D11Driver::getShaderByTypes(video::E_VERTEX_TYPE vertexType, E_D3D11_SHADER_TYPE shaderType, E_MATERIAL_TYPE materialType)查找Shader时，如果是VS，就通过vertexType查找，如果是PS，通过materialType查找
2. PS不需要根据E_VERTEX_TYPE来创建
3. PS根据E_MATERIAL_TYPE来创建
4. PS创建时需要加载PS_MaterialShaders.hlsl的对应shader
5. CD3D11Driver::setPSByType(video::E_MATERIAL_TYPE materialType, video::E_VERTEX_TYPE vertexType)改成仅用materialType来选择PS

# 50
Git commit: Fix HLSL build issue.
1. 把PS_MaterialShaders.hlsl的内容放到CD3D11MaterialRenderer，创建一个全局的字符串
2. CD3D11Driver::createMaterialPixelShader不需要file = FileSystem->createAndOpenFile("PS_MaterialShaders.hlsl");，直接加载该字符串

根据下面的报错和HLSL代码分析是什么问题
Shader@0x00007FF930B83630(36,11-19): error X4500: overlapping register semantics not yet implemented 't0'
float4 PS_SPHERE_MAP(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
    float3 reflectVec = reflect(-viewDir, input.Normal);
    float2 sphereUV = reflectVec.xy * 0.5 + 0.5;
    float4 sphereColor = SphereMap.Sample(LinearSampler, sphereUV);
    return texColor * sphereColor * 2.0;
}

# 51
Git commit: 
1. createRectangleShaders创建的VS/PS单独用一个成员变量保存
2. 创建一个set2DRectangleShader的函数来设置createRectangleShaders创建的VS/PS
3. draw2DRectangle调用set2DRectangleShader来设置VS/PS

# 52
Git commit: 