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
Git commit: Refine 2d rectangle shaders code by MiniMax-M2.7.
1. createRectangleShaders创建的VS/PS单独用一个成员变量保存
2. 创建一个set2DRectangleShader的函数来设置createRectangleShaders创建的VS/PS
3. draw2DRectangle调用set2DRectangleShader来设置VS/PS

CD3D11Driver::set2DRectangleShader不要使用m_BuiltInVSInitialized来决定是否初始化，创建一个新变量来判断
不要使用m_InputLayout[EVT_2D_RECTANGLE]来保存input layout，像shader一样，使用一个新变量保存。

# 52
Git commit: Add debugging code by MiniMax-M2.7.
d3d9的EMT_LIGHTMAP_M4实现如下，请帮我们分析其对应的PS实现
                m_pID3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
                m_pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
                m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
                m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
                m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE)

dump image in CD3D11Texture::createTexture

把下列调试使用的宏放到一个统一的CD3D11Debug.h头文件中，并添加注释，方便统一开启
_IRR_TEXTURE_DUMP
_IRR_MATERIAL_PRINT
_IRR_DUMP_DRAW_CALLS_FILE
_IRR_DUMP_DRAW_CALLS_PRINT
_IRR_DUMP_DRAW_CALLS_
_IRR_D3D11_OBJECT_TRACKING

# 53
Git commit: Add debug code for texture dumping.
CD3D11Driver::setMaterial打印当前Material用到的纹理

dump texture的同时，打印出dump的文件名
#define _IRR_DUMP_TEXTURE(img, name) \
    do { \
        core::stringc dumpName = "dump_"; \
        dumpName += name; \
        m_Driver->writeImageToFile(img, dumpName); \
    } while (false)
#else
#define _IRR_DUMP_TEXTURE(img, name) do { } while (false)
#endif

打印时给"dump_*"加上一个counter计数,文件名变成"dump_count_*"
#define _IRR_DUMP_TEXTURE(img, name) \
    do { \
        core::stringc dumpName = "dump_"; \
        dumpName += name; \
        os::Printer::log("DUMP_TEXTURE", dumpName.c_str()); \
        m_Driver->writeImageToFile(img, dumpName); \
    } while (false)
#else
#define _IRR_DUMP_TEXTURE(img, name) do { } while (false)
#endif

#define _IRR_DUMP_TEXTURE(img, name) \
    do { \
        core::stringc dumpName = "dump_"; \
        dumpName += core::stringc(CD3D11Texture::TextureDumpCounter); \
        dumpName += "_"; \
        dumpName += name; \
        os::Printer::log("DUMP_TEXTURE", dumpName.c_str()); \
        m_Driver->writeImageToFile(img, dumpName); \
        ++CD3D11Texture::TextureDumpCounter; \
    } while (false)
#else
#define _IRR_DUMP_TEXTURE(img, name) do { } while (false)
#endif
1. 检查name是否包含“#”，如果包含，用“-”替代“#”
2. 上面代码需要判断m_Driver->writeImageToFile(img, dumpName)的返回
3. 如果成功就继续往下执行
4. 如果失败，则检查文件名是否没有后缀名.bmp, .jpg, .pcx, .png, .pcm, .tga, .ppd，如果没有，就增加后缀名.jpg，重新调用m_Driver->writeImageToFile

下列代码，
1. 判断第一次dumpSuccess是否成功
2. 如果fail，则删除已经创建的file
#define _IRR_DUMP_TEXTURE(img, name)                                      \
    do {                                                                  \
        core::stringc    dumpName = "dump_";                              \
        dumpName    += core::stringc(CD3D11Texture::TextureDumpCounter);  \
        dumpName    += "_";                                               \
        dumpName    += name;                                              \
        dumpName.replace('#', '-');                                       \
        os::Printer::log("DUMP_TEXTURE", dumpName.c_str());               \
        bool    dumpSuccess = m_Driver->writeImageToFile(img, dumpName);  \
        if (!dumpSuccess)                                                 \
        {                                                                 \
            if (dumpName.find(".bmp") < 0 && dumpName.find(".jpg") < 0 && \
                dumpName.find(".pcx") < 0 && dumpName.find(".png") < 0 && \
                dumpName.find(".ppm") < 0 && dumpName.find(".tga") < 0 && \
                dumpName.find(".psd") < 0)                                \
            {                                                             \
                dumpName += ".jpg";                                       \
                os::Printer::log("DUMP_TEXTURE_RETRY", dumpName.c_str()); \
                dumpSuccess = m_Driver->writeImageToFile(img, dumpName);  \
            }                                                             \
        }                                                                 \
        if (dumpSuccess)                                                  \
            ++CD3D11Texture::TextureDumpCounter;                          \
    } while (false)

# 54
Git commit: Create sampler based on material's texture layer by MiniMax-M2.7.
我们对所有的纹理采样，d3d11都使用m_DefaultSampler。
1. 我们需要根据Material内的SMaterialLayer    TextureLayer来判断某一个纹理需要使用什么样的采样器
   请根据SMaterialLayer提供的下列参数来创建对应的CSampler
            //! Texture Clamp Mode
            /** Values are taken from E_TEXTURE_CLAMP. */
            u8 TextureWrapU : 4;
            u8 TextureWrapV : 4;

            //! Is bilinear filtering enabled? Default: true
            bool BilinearFilter : 1;

            //! Is trilinear filtering enabled? Default: false
            /** If the trilinear filter flag is enabled,
             * the bilinear filtering flag is ignored. */
            bool TrilinearFilter : 1;

            //! Is anisotropic filtering enabled? Default: 0, disabled
            /** In Irrlicht you can use anisotropic texture filtering
             * in conjunction with bilinear or trilinear texture
             * filtering to improve rendering results. Primitives
             * will look less blurry with this flag switched on. The number gives
             * the maximal anisotropy degree, and is often in the range 2-16.
             * Value 1 is equivalent to 0, but should be avoided. */
            u8 AnisotropicFilter;

            //! Bias for the mipmap choosing decision.
            /** This value can make the textures more or less blurry than with the
             * default value of 0. The value (divided by 8.f) is added to the mipmap level
             * chosen initially, and thus takes a smaller mipmap for a region
             * if the value is positive. */
            s8 LODBias;
2. 创建的CSampler都使用一个m_SamplerPool进行管理
3. 创建CSampler之前，请根据SMaterialLayer参数判断在m_SamplerPool是否已经有已经创建的sampler，如果有则直接使用已有的。
4. 创建一个m_CurrentSampler[MATERIAL_MAX_TEXTURES]保存当前纹理单元使用的CSampler对象
5. CD3D11Driver::setPSTextureAndSamplerState配置CSampler时，使用m_CurrentSampler

# 55
Git commit: Fix sampler/texture settings if changing material's texture, not changing material's type by MiniMax-M2.7.
CD3D11Driver::setPSTextureAndSamplerState总是根据当前m_CurrentTexture和m_CurrentSampler的状态来设置texture/sampler。
我们需要根据当前texture/sampler与上一次draw有没有变化来决定是否设置

# 56
Git commit: Create render states based on material step1 by MiniMax-M2.7.
d3d11的m_RenderStateSets[ERM_RENDER_MODE_MAX]只是针对每个render mode创建一套pipeline state，并不能满足不同material的要求。请做如下修改
1. 每个render mode可以有多个render states，每个render states根据material的配置来创建
2. m_RenderStateSets[ERM_RENDER_MODE_MAX]变成一个字典数组，每个创建的SRenderStateSet对象同时创建一个key，(key, SRenderStateSet)都保存在m_RenderStateSets[ERM_RENDER_MODE_MAX]中。
3. depth/stencil/rasterizer/blend的状态的创建需要参考material的设置
4. 每次需要创建SRenderStateSet前，在m_RenderStateSets[ERM_RENDER_MODE_MAX]查找是否有已经创建的state使用，如果没有则创建
5. 删除SRenderStateSet中的AlphaBlendState，每个blend state的创建要根据material的配置，
   A. 如果是ERM_2D，根据setRenderStates2DMode的输入参数满足下面条件就开启alphablend
            alphaChannel &= texture;

            if (alpha || alphaChannel)
            {
                // Enable alpha blend
            }
            else
            {
                // Disable alpha blend
            }
    B. 如果是ERM_3D，则根据material的“E_BLEND_OPERATION    BlendOperation”来确定是否开启，以及alpha blend的设置参数，可以参考如下d3d9代码
            if (queryFeature(EVDF_BLEND_OPERATIONS) &&
                (resetAllRenderstates || lastmaterial.BlendOperation != material.BlendOperation))
            {
                if (material.BlendOperation == EBO_NONE)
                    m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
                else
                {
                    m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

                    switch (material.BlendOperation)
                    {
                        case EBO_SUBTRACT:
                            m_pID3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT);
                            break;

                        case EBO_REVSUBTRACT:
                            m_pID3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
                            break;

                        case EBO_MIN:
                        case EBO_MIN_FACTOR:
                        case EBO_MIN_ALPHA:
                            m_pID3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MIN);
                            break;

                        case EBO_MAX:
                        case EBO_MAX_FACTOR:
                        case EBO_MAX_ALPHA:
                            m_pID3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MAX);
                            break;

                        default:
                            m_pID3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
                            break;
                    }
                }
            }

6. 帮我解析这段代码的意思，以及对应于d3d11的实现，看起来似乎d3d11需要用shader来完成
            if (lastmaterial.ColorMaterial != material.ColorMaterial)
            {
                m_pID3DDevice->SetRenderState(D3DRS_COLORVERTEX, (material.ColorMaterial != ECM_NONE));
                m_pID3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,
                                              ((material.ColorMaterial == ECM_DIFFUSE) ||
                                               (material.ColorMaterial == ECM_DIFFUSE_AND_AMBIENT)) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
                m_pID3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,
                                              ((material.ColorMaterial == ECM_AMBIENT) ||
                                               (material.ColorMaterial == ECM_DIFFUSE_AND_AMBIENT)) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
                m_pID3DDevice->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,
                                              (material.ColorMaterial == ECM_EMISSIVE) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
                m_pID3DDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,
                                              (material.ColorMaterial == ECM_SPECULAR) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
            }

7. CD3D11Driver::createRenderStateKey创建key，针对ERM_2D,ERM_3D和其他rendermode分别创建，
8. ERM_2D使用mode, alpha, texture, alphaChannel来创建key
9.  ERM_3D使用SMaterial &material内的参数来创建key，包括以下参数
            //! Type of the material. Specifies how everything is blended together
            E_MATERIAL_TYPE    MaterialType;

            //! Free parameter, dependent on the material type.
            /** Mostly ignored, used for example in EMT_PARALLAX_MAP_SOLID
             * and EMT_TRANSPARENT_ALPHA_CHANNEL. */
            f32    MaterialTypeParam;

            //! Thickness of non-3dimensional elements such as lines and points.
            f32    Thickness;

            //! Is the ZBuffer enabled? Default: ECFN_LESSEQUAL
            /** Values are from E_COMPARISON_FUNC. */
            u8    ZBuffer;

            //! Sets the antialiasing mode
            /** Values are chosen from E_ANTI_ALIASING_MODE. Default is
             * EAAM_SIMPLE|EAAM_LINE_SMOOTH, i.e. simple multi-sample
             * anti-aliasing and lime smoothing is enabled. */
            u8    AntiAliasing;

            //! Defines the enabled color planes
            /** Values are defined as or'ed values of the E_COLOR_PLANE enum.
             * Only enabled color planes will be rendered to the current render
             * target. Typical use is to disable all colors when rendering only to
             * depth or stencil buffer, or using Red and Green for Stereo rendering. */
            u8    ColorMask : 4;

            //! Store the blend operation of choice
            /** Values to be chosen from E_BLEND_OPERATION. The actual way to use this value
             * is not yet determined, so ignore it for now. */
            E_BLEND_OPERATION    BlendOperation : 4;

            //! Factor specifying how far the polygon offset should be made
            /** Specifying 0 disables the polygon offset. The direction is specified spearately.
             * The factor can be from 0 to 7.*/
            u8    PolygonOffsetFactor : 3;

            //! Flag defining the direction the polygon offset is applied to.
            /** Can be to front or to back, specififed by values from E_POLYGON_OFFSET. */
            E_POLYGON_OFFSET    PolygonOffsetDirection : 1;

            //! Draw as wireframe or filled triangles? Default: false
            /** The user can access a material flag using
             * \code material.Wireframe=true \endcode
             * or \code material.setFlag(EMF_WIREFRAME, true); \endcode */
            bool    Wireframe : 1;

            //! Draw as point cloud or filled triangles? Default: false
            bool    PointCloud : 1;

            //! Is the zbuffer writeable or is it read-only. Default: true.
            /** This flag is forced to false if the MaterialType is a
             * transparent type and the scene parameter
             * ALLOW_ZWRITE_ON_TRANSPARENT is not set. */
            bool    ZWriteEnable : 1;

            //! Is backface culling enabled? Default: true
            bool    BackfaceCulling : 1;

            //! Is frontface culling enabled? Default: false
            bool    FrontfaceCulling : 1;

# 58
Git commit: Create render states based on material step2. Now rendering is OK. The coding is by MiniMax-M2.7.
1. CD3D11Driver::getOrCreateRenderStateSet分成三个函数，分别针对ERM_3D，ERM_2D和其他render mode
2. 把ERM_3D,ERM_2D和其它render mode的key生成代码用函数完成，不要在代码里分散计算。

# 59
Git commit: Fix memory leak caused by rectangle shader reference by MiniMax-M2.7.
下列d3d11 object没有释放
            ID3D11VertexShader              *m_RectangleVertexShader;
            ID3D11PixelShader               *m_RectanglePixelShader;
            ID3D11InputLayout               *m_RectangleInputLayout;

# 60
Git commit: Fix vs/ps input/output mismatch issue. Sometimes ps's material is EMT_SOLID and vs is EVT_2TCOORDS, but there is one texture. Fixed by ying.
代码#define _IRR_DUMP_TEXTURE(img, name) name可能是包含了全路径的pathname，如下所示，请去掉路劲
DUMP_TEXTURE: dump_1_C:/Development/Graphics/irrlicht-1.8.3/media/wall.bmp
改为
DUMP_TEXTURE: dump_1_wall.bmp

1. 看起来name没有getFileName函数，请直接用字符串处理的办法去掉路径，只保留文件名字
错误	C2039	"getFileName": 不是 "irr::io::SNamedPath" 的成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	347		
错误(活动)	E0135	类 "irr::io::SNamedPath" 没有成员 "getFileName"	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp	347		
2. 在调用 m_Driver->writeImageToFile(img, dumpName)前
   A. 如果name包含‘#’替代为‘-’
   B. 如果name的后缀名不是.bmp, .jpg, .pcx, .png, .pcm, .tga, .ppd，就增加后缀名.jpg

#define _IRR_DUMP_TEXTURE(img, name)把tga后缀名换成jpg

# 61
Git commit: Implement draw2DImage for Movement example by MiniMax-M2.7.
调用CD3D11Driver::draw2DImageBatch
实现下列函数
        void CD3D11Driver::draw2DImage(const video::ITexture *texture, const core::position2d<s32> &destPos,
                                       const core::rect<s32> &sourceRect, const core::rect<s32> *clipRect,
                                       SColor color, bool useAlphaChannelOfTexture)
        {}


        void CD3D11Driver::draw2DImage(const video::ITexture *texture, const core::rect<s32> &destRect,
                                       const core::rect<s32> &sourceRect, const core::rect<s32> *clipRect,
                                       const video::SColor* const colors, bool useAlphaChannelOfTexture)
        {}

# 62
Git commit: We should check 2d render state key to decide if there is 2d render state switch by MiniMax-M2.7.
setRenderStates2DMode函数不能简单判断上次是否是ERM_2D,应该通过SRenderStateSet的key来判断当前ERM_2D的state是否跟上次一样，如果不一样则配置新的state。
        void CD3D11Driver::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)
        {
            if (m_CurrentRenderMode == ERM_2D)
                return;

# 63
Git commit: Implement texture lock/unlock by MiniMax-M2.7.
实现下面函数
        void* CD3D11Texture::lock(E_TEXTURE_LOCK_MODE mode, u32 mipmapLevel)
        {
            return 0;
        }


        void CD3D11Texture::unlock()
        {}
1. 如果texture的video memory能够被CPU access，直接返回lock地址，
2. 如果texture的video memory不能被CPU access，则创建一个临时resource，该resource需要能CPU access
   a. 在Lock时把当前texture内容拷贝到临时resource中,返回resource的cpu地址
   b. 在unlock时把resource中的内容又拷贝回当前texture中。

3. CD3D11Texture::lock中创建的m_StagingTexture只需要在第一次lock时被创建出来，后面就一直使用，在CD3D11Texture对象被销毁时再销毁m_StagingTexture
4. CD3D11Texture::lock已经实现ETLM_READ_WRITE，ETLM_READ_ONLY，没有实现ETLM_WRITE_ONLY

# 64
Git commit: 1. Fix wrong last ps material type. 2. print draw parameters.
函数dumpDrawCall打印vType，pType，iType不要当成整型参数打印，把其对应的enmu名称打印出来，is3D也要打印成字符串
        void CD3D11Driver::dumpDrawCall(const c8 *drawTypeName, E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
            E_INDEX_TYPE iType, bool is3D, u32 primitiveCount)
        {
            ++DrawCallCounter;

            char    paramStr[256];
            sprintf(paramStr, "%s: v%d_p%d_i%d_3d%d_pc%d", drawTypeName,
                vType, pType, iType, is3D ? 1 : 0, primitiveCount);

#if _IRR_DUMP_DRAW_CALLS_PRINT
            os::Printer::log("DrawCall", core::stringc(DrawCallCounter).c_str(), ELL_INFORMATION);
            os::Printer::log(paramStr);
#endif

#if _IRR_DUMP_DRAW_CALLS_FILE
            IImage    *image = createScreenShot(ECOLOR_FORMAT::ECF_A8R8G8B8, video::ERT_FRAME_BUFFER);
            if (image)
            {
                core::stringc    filename = "draw_";
                filename    += DrawCallCounter;
                filename    += "_";
                filename    += drawTypeName;
                filename    += ".jpg";
                writeImageToFile(image, filename.c_str(), 90);
                image->drop();
            }
#endif
        }

# 65
Git commit: Fix 3d render state switch. We should use both mode and key. by MiniMax-M2.7.
1. 把CD3D11Driver::setRenderStates的代码合并到CD3D11Driver::setRenderStates3DMode()，删除CD3D11Driver::setRenderStates
2. 判断是否需要重新设置3d render states,不能只是判断render mode，还需要结合当前render state key来判断。

# 66
Git commit: Fix 3d key collision by MiniMax-M2.7.
CD3D11Driver::createRenderStateKey3D计算key时，material.MaterialTypeParam和material.Thickness都是32bit，因此会覆盖高32位的key值。
请为material.MaterialTypeParam和material.Thickness单独创建一个u64 key。3d render states使用两个key进行比较，2d还是使用原来的一个key。
            return u64(ERM_3D) |
                   (u64(material.MaterialType) << 4) |
                   (u64(*(u32*)&material.MaterialTypeParam) << 12) |
                   (u64(*(u32*)&material.Thickness) << 20) |
                   (u64(material.ZBuffer) << 28) |
                   (u64(material.AntiAliasing) << 32) |
                   (u64(material.ColorMask) << 36) |
                   (u64(material.BlendOperation) << 40) |
                   (u64(material.PolygonOffsetFactor) << 44) |
                   (u64(material.PolygonOffsetDirection) << 48) |
                   (u64(material.Wireframe) << 52) |
                   (u64(material.PointCloud) << 53) |
                   (u64(material.ZWriteEnable) << 54) |
                   (u64(material.BackfaceCulling) << 55) |
                   (u64(material.FrontfaceCulling) << 56);

下列material的成员变量值，为什么计算CD3D11Driver::createRenderStateKey3D时，key1的(u64(material.Wireframe) << 52)位是1，而material.Wireframe的值是false
-		material	{TextureLayer=0x0000027446cbbbd8 {{Texture=0x0000027451481580 {...} TextureWrapU='\0' TextureWrapV='\0' ...}, ...} ...}	const irr::video::SMaterial &
+		TextureLayer	0x0000027446cbbbd8 {{Texture=0x0000027451481580 {m_Device=0x0000027446cf0fd0 {...} m_Texture=0x000002744eee35d0 {...} ...} ...}, ...}	irr::video::SMaterialLayer[0x00000008]
		MaterialType	EMT_LIGHTMAP_M4 (0x00000005)	irr::video::E_MATERIAL_TYPE
+		AmbientColor	{color=0xffffffff }	irr::video::SColor
+		DiffuseColor	{color=0xffffffff }	irr::video::SColor
+		EmissiveColor	{color=0x00000000 }	irr::video::SColor
+		SpecularColor	{color=0xffffffff }	irr::video::SColor
		Shininess	0.00000000	float
		MaterialTypeParam	0.00000000	float
		MaterialTypeParam2	-1.00000000	float
		Thickness	1.00000000	float
		ZBuffer	0x01 '\x1'	unsigned char
		AntiAliasing	0x01 '\x1'	unsigned char
		ColorMask	0x0f '\xf'	unsigned char
		ColorMaterial	0x01 '\x1'	unsigned char
		BlendOperation	EBO_NONE (0x00000000)	irr::video::E_BLEND_OPERATION
		PolygonOffsetFactor	0x00 '\0'	unsigned char
		PolygonOffsetDirection	EPO_FRONT | 0xfffffffe (0xffffffff)	irr::video::E_POLYGON_OFFSET
		Wireframe	false	bool
		PointCloud	false	bool
		GouraudShading	true	bool
		Lighting	false	bool
		ZWriteEnable	true	bool
		BackfaceCulling	true	bool
		FrontfaceCulling	false	bool
		FogEnable	false	bool
		NormalizeNormals	false	bool
		UseMipMaps	true	bool


# 67
Git commit: Add antialise and colormask setting for 3d render states by MiniMax-M2.7.
1. 根据material的AntiAliasing设置，来配置d3d11的rasterizer state。
            //! Sets the antialiasing mode
            /** Values are chosen from E_ANTI_ALIASING_MODE. Default is
             * EAAM_SIMPLE|EAAM_LINE_SMOOTH, i.e. simple multi-sample
             * anti-aliasing and lime smoothing is enabled. */
            u8    AntiAliasing;

        enum E_ANTI_ALIASING_MODE
        {
            //! Use to turn off anti-aliasing for this material
            EAAM_OFF = 0,
            //! Default anti-aliasing mode
            EAAM_SIMPLE = 1,
            //! High-quality anti-aliasing, not always supported, automatically enables SIMPLE mode
            EAAM_QUALITY = 3,
            //! Line smoothing
            EAAM_LINE_SMOOTH = 4,
            //! point smoothing, often in software and slow, only with OpenGL
            EAAM_POINT_SMOOTH = 8,
            //! All typical anti-alias and smooth modes
            EAAM_FULL_BASIC = 15,
            //! Enhanced anti-aliasing for transparent materials
            /** Usually used with EMT_TRANSPARENT_ALPHA_REF and multisampling. */
            EAAM_ALPHA_TO_COVERAGE = 16
        };
2. 3d key计算，看起来material.AntiAliasing左移32位，material.AntiAliasing左移36位，而material.AntiAliasing是8位，有可能覆盖掉material.AntiAliasing，请解决这个冲突
                   (u64(material.AntiAliasing) << 32) |
                   (u64(material.ColorMask) << 36) |
3. 根据material.ColorMask的配置，来设置blend state的RenderTargetWriteMask
4. 根据material.AntiAliasing是否设置alpha to coverage来配置blend state的AlphaToCoverageEnable


# 68
Git commit: Fix wrong color swizzle between D3D9 and D3D11 by ying.
CNullDriver::draw3DTriangle(const core::triangle3df &triangle, SColor color)输入的color从高位到低位是ARGB，但是d3d11对顶点color的排序从高到低是ABGR,请在CNullDriver::draw3DTriangle函数，针对D3D11的编译，对颜色做swizzle.
        // ! Draws a 3d triangle.
        void CNullDriver::draw3DTriangle(const core::triangle3df &triangle, SColor color)
        {
            S3DVertex    vertices[3];

            vertices[0].Pos     = triangle.pointA;
            vertices[0].Color   = color;
            vertices[0].Normal  = triangle.getNormal().normalize();
            vertices[0].TCoords.set(0.f, 0.f);
            vertices[1].Pos     = triangle.pointB;
            vertices[1].Color   = color;
            vertices[1].Normal  = vertices[0].Normal;
            vertices[1].TCoords.set(0.5f, 1.f);
            vertices[2].Pos     = triangle.pointC;
            vertices[2].Color   = color;
            vertices[2].Normal  = vertices[0].Normal;
            vertices[2].TCoords.set(1.f, 0.f);
            const u16    indexList[] = {0, 1, 2};
            drawVertexPrimitiveList(vertices, 3, indexList, 1, EVT_STANDARD, scene::EPT_TRIANGLES, EIT_16BIT);
        }

            SColor (u32 a, u32 r, u32 g, u32 b)
                : color(((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff)) {}


# 69
Git commit: Fix alpha blend enable if using transparent material by MiniMax-M2.7.
material.MaterialType为EMT_TRANSPARENT_ADD_COLOR时，d3d9要做如下配置
                    setTextureColorStage(m_pID3DDevice, 0,
                        D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);

                    m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
                    m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                    m_pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
                    m_pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);

EMT_TRANSPARENT_ADD_COLOR的描述如下
            //! A transparent material.
            /** Only the first texture is used. The new color is calculated
             * by simply adding the source color and the dest color. This
             * means if for example a billboard using a texture with black
             * background and a red circle on it is drawn with this material,
             * the result is that only the red circle will be drawn a little
             * bit transparent, and everything which was black is 100%
             * transparent and not visible. This material type is useful for
             * particle effects. */
根据以上的信息，对d3d11做相应更改。d3d11如果material type是EMT_TRANSPARENT_ADD_COLOR
1. 开启alpha blend
2. src blend是one，dest blend是INVSRCCOLOR

material.MaterialType为EMT_TRANSPARENT_ALPHA_CHANNEL时，d3d9要做如下配置
                    m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
                    m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                    m_pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
                    m_pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

                    m_pID3DDevice->SetRenderState(D3DRS_ALPHAREF, core::floor32(material.MaterialTypeParam * 255.f));
                    m_pID3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
                    m_pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);

EMT_TRANSPARENT_ALPHA_CHANNEL的描述如下
             * The final color is blended together from the destination
             * color and the texture color, using the alpha channel value as
             * blend factor. Only first texture is used. If you are using
             * this material with small textures, it is a good idea to load
             * the texture in 32 bit mode
             * (video::IVideoDriver::setTextureCreationFlag()). Also, an alpha
             * ref is used, which can be manipulated using
             * SMaterial::MaterialTypeParam. This value controls how sharp the
             * edges become when going from a transparent to a solid spot on
             * the texture
根据以上的信息，对d3d11做相应更改。

EMT_TRANSPARENT_ALPHA_CHANNEL_REF的描述如下
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
根据上述的描述，d3d11的PS_TRANSPARENT_ALPHA_CHANNEL shader需要考虑其最后output的color的alpha channel值，如果大于127，这个Pixel就输出，如果小于等于127，就抛弃这个pixel。

# 70
Git commit: Implement gouraud shading PS by MiniMax-M2.7.
1. 新增material type EMT_SOLID_LIGHTING_GOURAUD
2. 在EMT_SOLID对应的PS实现基础上实现EMT_SOLID_LIGHTING_GOURAUD，主要实现Gouraud shading
float4 PS_SOLID(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);
}
3. d3d9的Gouraud shading是这样实现m_pID3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);请参考来实现PS

# 71
Git commit: Implement lighting+FLAT PS by MiniMax-M2.7.
仿照EMT_SOLID_LIGHTING_GOURAUD和PS_SOLID_LIGHTING_GOURAUD实现，
实现EMT_SOLID_LIGHTING_FLAT和PS_SOLID_LIGHTING_FLAT。
请参照git commit id: 87d9da4679c42db55a6627d3ceb2c427cd960576

# 72
Git commit: Implement gouraud shading based on Lights by MiniMax-M2.7.
1. CD3D11Driver::turnLightOn并不是只开启一个light,而是需要根据每次传入的index，开启对应的light
2. EMT_SOLID_LIGHTING_GOURAUD和PS_SOLID_LIGHTING_GOURAUD实现需要更改，要依赖Light的 E_LIGHT_TYPE Type生成ELT_POINT，ELT_SPOT，ELT_DIRECTIONAL三种不同的灯光PS。
3. 对于CD3D11MaterialRenderer.h文件上实现的PS_SOLID_LIGHTING_GOURAUD，需要设置其ambient为m_AmbientLight表示的值。
4. 对于PS_SOLID_LIGHTING_GOURAUD,根据CD3D11Driver::turnLightOn开启的Light，从core::array<SLight> Lights中找到对应的Light，根据以下Light参数来设置对应的PS内的控制变量
               //! Ambient color emitted by the light
            SColorf AmbientColor;

            //! Diffuse color emitted by the light.
            /** This is the primary color you want to set. */
            SColorf DiffuseColor;

            //! Specular color emitted by the light.
            /** For details how to use specular highlights, see SMaterial::Shininess */
            SColorf SpecularColor;

            //! Attenuation factors (constant, linear, quadratic)
            /** Changes the light strength fading over distance.
             * Can also be altered by setting the radius, Attenuation will change to
             * (0,1.f/radius,0). Can be overridden after radius was set. */
            core::vector3df Attenuation;

            //! The angle of the spot's outer cone. Ignored for other lights.
            f32 OuterCone;

            //! The angle of the spot's inner cone. Ignored for other lights.
            f32 InnerCone;

            //! The light strength's decrease between Outer and Inner cone.
            f32 Falloff;

            //! Read-ONLY! Position of the light.
            /** If Type is ELT_DIRECTIONAL, it is ignored. Changed via light scene node's position. */
            core::vector3df Position;

            //! Read-ONLY! Direction of the light.
            /** If Type is ELT_POINT, it is ignored. Changed via light scene node's rotation. */
            core::vector3df Direction;

            //! Read-ONLY! Radius of light. Everything within this radius will be lighted.
            f32 Radius;

            //! Read-ONLY! Type of the light. Default: ELT_POINT
            E_LIGHT_TYPE Type;

            //! Read-ONLY! Does the light cast shadows?
            bool CastShadows : 1;

PS_SOLID_LIGHTING_GOURAUD的ambient，diffuse，lighting都改成float4
    float ambient = LightAmbient.r;
    float diffuse = max(dot(normal, lightDir), 0.0f);
    float lighting = ambient + diffuse * attenuation;

my fix:
1. PS_SOLID_LIGHTING_FLAT: dFdx/dFdy -> ddx/ddy

# 73
Git commit: Implement gouraud shading of VS for EVT_STANDARD.
根据基础的VERTEX_SHADER_STANDARD内容以及SLight的下列参数，生成三组新的VS，分别支持ELT_POINT，ELT_SPOT，ELT_DIRECTIONAL三种不同的灯光
               //! Ambient color emitted by the light
            SColorf AmbientColor;

            //! Diffuse color emitted by the light.
            /** This is the primary color you want to set. */
            SColorf DiffuseColor;

            //! Specular color emitted by the light.
            /** For details how to use specular highlights, see SMaterial::Shininess */
            SColorf SpecularColor;

            //! Attenuation factors (constant, linear, quadratic)
            /** Changes the light strength fading over distance.
             * Can also be altered by setting the radius, Attenuation will change to
             * (0,1.f/radius,0). Can be overridden after radius was set. */
            core::vector3df Attenuation;

            //! The angle of the spot's outer cone. Ignored for other lights.
            f32 OuterCone;

            //! The angle of the spot's inner cone. Ignored for other lights.
            f32 InnerCone;

            //! The light strength's decrease between Outer and Inner cone.
            f32 Falloff;

            //! Read-ONLY! Position of the light.
            /** If Type is ELT_DIRECTIONAL, it is ignored. Changed via light scene node's position. */
            core::vector3df Position;

            //! Read-ONLY! Direction of the light.
            /** If Type is ELT_POINT, it is ignored. Changed via light scene node's rotation. */
            core::vector3df Direction;

            //! Read-ONLY! Radius of light. Everything within this radius will be lighted.
            f32 Radius;

            //! Read-ONLY! Type of the light. Default: ELT_POINT
            E_LIGHT_TYPE Type;

            //! Read-ONLY! Does the light cast shadows?
            bool CastShadows : 1;

CD3D11Driver::setVSByVertexType在vType设置成EVT_STANDARD时，需要对m_Material.Lighting做光照处理
1. 如果Light为false，就用EVT_STANDARD的VS
2. 如果Light为true, 获取当前turn on light，根据light类型，选择下列对应的VS
    EVT_STANDARD_LIGHTING_DIRECTIONAL,
    EVT_STANDARD_LIGHTING_SPOT,
    EVT_STANDARD_LIGHTING_POINT,

CD3D11Driver::setVSByVertexType如果需要使用下列带光照的VS
    EVT_STANDARD_LIGHTING_DIRECTIONAL,
    EVT_STANDARD_LIGHTING_SPOT,
    EVT_STANDARD_LIGHTING_POINT,
1. 需要判断是VS还是PS开启了光照计算
2. 如果是VS开启，则在CD3D11Driver::setVSByVertexType中设置m_LightConstantBuffer到CBV
3. 如果是PS开启，则在CD3D11Driver::setPSByMaterialType中设置m_LightConstantBuffer到CBV
4. 不要直接在CD3D11Driver::turnLightOn中设置m_LightConstantBuffer。

# 74
Git commit: Implement gouraud shading of VS for EVT_2TCOORDS by MiniMax-M2.7.
1. 仿照EVT_STANDARD+gouraud shading实现的下列shader，为EVT_2TCOORDS增加gourand shading
    EVT_STANDARD_LIGHTING_POINT;
    EVT_STANDARD_LIGHTING_SPOT;
    EVT_STANDARD_LIGHTING_DIRECTIONAL;
2. CD3D11Driver::setVSByVertexType,在vType == EVT_2TCOORDS && m_Material.GouraudShading时，也选择对应的gouraud shading版本的VS

Delete the following code of S3DVertex.h
{
            //! default constructor
            S3DVertex() {}


# 75
Git commit: Add material color to PS by MiniMax-M2.7.
帮我分析下面d3d9的代码的功能，如果用d3d11实现，需要更改的是VS还是PS，以及具体的更改的方法。
            {
                D3DMATERIAL9    mat;
                mat.Diffuse     = colorToD3D(material.DiffuseColor);
                mat.Ambient     = colorToD3D(material.AmbientColor);
                mat.Specular    = colorToD3D(material.SpecularColor);
                mat.Emissive    = colorToD3D(material.EmissiveColor);
                mat.Power       = material.Shininess;
                m_pID3DDevice->SetMaterial(&mat);
            }

            {
                m_pID3DDevice->SetRenderState(D3DRS_COLORVERTEX, (material.ColorMaterial != ECM_NONE));
                m_pID3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,
                                              ((material.ColorMaterial == ECM_DIFFUSE) ||
                                               (material.ColorMaterial == ECM_DIFFUSE_AND_AMBIENT)) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
                m_pID3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,
                                              ((material.ColorMaterial == ECM_AMBIENT) ||
                                               (material.ColorMaterial == ECM_DIFFUSE_AND_AMBIENT)) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
                m_pID3DDevice->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,
                                              (material.ColorMaterial == ECM_EMISSIVE) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
                m_pID3DDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,
                                              (material.ColorMaterial == ECM_SPECULAR) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
            }

根据前面分析的结果，我们需要更改下列的PS，使得PS可以支持
1. 材质数据传入pixel shader（通过 Constant Buffer）,使得PS可以选择使用color
// cbuffer 中定义材质
cbuffer MaterialBuffer : register(b2)
{
    float4 DiffuseColor;
    float4 AmbientColor;
    float4 SpecularColor;
    float4 EmissiveColor;
    float  Shininess;
    float3 Padding;
    int    ColorMaterialMode; // 0=全部用材质, 1=用顶点颜色等
};

float4 PS_SOLID_1_LAYER(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);
}

float4 PS_SOLID(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);
}
2. CD3D11Driver::setBasicRenderStates需要类似CD3D9Driver::setBasicRenderStates，把下列参数传递给PS
               material.DiffuseColor
               material.AmbientColor
               material.SpecularColor
               material.EmissiveColor
               material.Shininess

# 76
Git commit: 
根据下面代码不同的ColorMaterial设置的d3d9渲染管线，请告诉我pixel color的计算公式
            if (lastmaterial.ColorMaterial != material.ColorMaterial)
            {
                m_pID3DDevice->SetRenderState(D3DRS_COLORVERTEX, (material.ColorMaterial != ECM_NONE));
                m_pID3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,
                                              ((material.ColorMaterial == ECM_DIFFUSE) ||
                                               (material.ColorMaterial == ECM_DIFFUSE_AND_AMBIENT)) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
                m_pID3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,
                                              ((material.ColorMaterial == ECM_AMBIENT) ||
                                               (material.ColorMaterial == ECM_DIFFUSE_AND_AMBIENT)) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
                m_pID3DDevice->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,
                                              (material.ColorMaterial == ECM_EMISSIVE) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
                m_pID3DDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,
                                              (material.ColorMaterial == ECM_SPECULAR) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
            }

根据前面的颜色公式重写下面的两个PS
float4 PS_SOLID(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);

    float4 diffuse, ambient, specular, emissive;

    if (ColorMaterialMode == ECM_DIFFUSE || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        diffuse = input.Color;
    else
        diffuse = DiffuseColor;

    if (ColorMaterialMode == ECM_AMBIENT || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        ambient = input.Color;
    else
        ambient = AmbientColor;

    if (ColorMaterialMode == ECM_SPECULAR)
        specular = input.Color;
    else
        specular = SpecularColor;

    if (ColorMaterialMode == ECM_EMISSIVE)
        emissive = input.Color;
    else
        emissive = EmissiveColor;

    float4 finalColor = texColor * diffuse;
    finalColor.rgb *= ambient.rgb;
    finalColor.rgb += emissive.rgb;

    return float4(finalColor.rgb * input.Color.a, texColor.a * input.Color.a);
}

float4 PS_SOLID_1_LAYER(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);

    float4 diffuse, ambient, specular, emissive;

    if (ColorMaterialMode == ECM_DIFFUSE || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        diffuse = input.Color;
    else
        diffuse = DiffuseColor;

    if (ColorMaterialMode == ECM_AMBIENT || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        ambient = input.Color;
    else
        ambient = AmbientColor;

    if (ColorMaterialMode == ECM_SPECULAR)
        specular = input.Color;
    else
        specular = SpecularColor;

    if (ColorMaterialMode == ECM_EMISSIVE)
        emissive = input.Color;
    else
        emissive = EmissiveColor;

    float4 finalColor = texColor * diffuse;
    finalColor.rgb *= ambient.rgb;
    finalColor.rgb += emissive.rgb;

    return float4(finalColor.rgb * input.Color.a, texColor.a * input.Color.a);
}

注意标准光照公式 FinalColor = Emissive + Ambient×GlobalAmbient + Diffuse×(N·L) + Specular×(R·V)
N代表PS输入的Normal向量，L表示入射光方向，R是光线的反射向量，V是视线向量

# 77
Git commit: 

        // EVT_2TCOORDS with ELT_DIRECTIONAL Light Vertex Shader
        static const char    VERTEX_SHADER_2TCOORDS_DIRECTIONAL[] =
            "struct VS_INPUT {"
            "    float3 Pos : POSITION;"
            "    float3 Normal : NORMAL;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float2 TexCoord2 : TEXCOORD1;"
            "};"
            "struct VS_OUTPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float2 TexCoord2 : TEXCOORD1;"
            "    float3 Normal : NORMAL;"
            "    float3 WorldPos : TEXCOORD3;"
            "};"
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "    float4x4 World;"
            "};"
            "cbuffer LightBuffer : register(b1) {"
            "    float4 AmbientColor;"
            "    float4 DiffuseColor;"
            "    float4 SpecularColor;"
            "    float3 LightPos;"
            "    float LightRadius;"
            "    float3 LightDir;"
            "    float3 Attenuation;"
            "    float OuterCone;"
            "    float InnerCone;"
            "};"
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Normal = mul(input.Normal, (float3x3)World);"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord2;"
            "    float3 normal = normalize(input.Normal);"
            "    float diff = max(dot(normal, normalize(-LightDir.xyz)), 0.0);"
            "    output.Color = float4(AmbientColor.rgb + DiffuseColor.rgb * diff, 1.0);"
            "    return output;"
            "}";
上面这段VS编译报错如下，帮我修复
C:\Development\Graphics\irrlicht-1.8.3\examples\07.Collision\Shader@0x00007FF9E05C7840(1,1031-1060): error X3025: global variables are implicitly constant, enable compatibility mode to allow modification


# 78
Git commit: Refine SetConstant slot name and material PS by MiniMax-M2.7.
m_pID3DDeviceContext->PSSetConstantBuffers和m_pID3DDeviceContext->VSSetConstantBuffers的StartSlot和NumBuffers参数不要用数字，请用宏来替代

# 79
Git commit: Refine debug log functions by MiniMax-M2.7.
像 static inline const char* getColorMaterialName(E_COLOR_MATERIAL mode)实现一样，
把CD3D11Driver::getVertexTypeName, CD3D11Driver::getMaterialTypeName, CD3D11Driver::getLightTypeName, CD3D11Driver::getPrimitiveTypeName迁移到enum对应的头文件

# 80
good example
Git commit: Fix pixel color calcualtion based on texture, material, light by MiniMax-M2.7.
d3d9配置了SMaterial material和SLight light，请问这两个颜色配置是如何影响到pixel color

D3D9 固定功能光照公式
Diffuse = Material.DiffuseColor × Light.DiffuseColor × max(N·L, 0)
Specular = Material.SpecularColor × Light.SpecularColor × pow(max(R·V, 0), Material.Shininess)
Ambient = Material.AmbientColor × Light.AmbientColor
Emissive = Material.EmissiveColor
D3DTA_DIFFUSE = Diffuse + Specular + Ambient + Emissive
最终像素颜色（纹理阶段后）
PixelColor = TextureColor(u,v) × D3DTA_DIFFUSE
- N = 表面法线，L = 光源方向，R = 反射方向，V = 视点方向
- (N·L) 和 (R·V) 是点积，控制光照强度分布
请根据上面的结论和公式，帮我重写PS_SOLID
1. 需要使用到光源常量和材质常量
2. 需要根据材质的mode来判断是使用材质的颜色，还是input.Color
3. 还需要加上纹理。
float4 PS_SOLID(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);

    float4 diffuse, ambient, specular, emissive;

    if (ColorMaterialMode == ECM_DIFFUSE || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        diffuse = input.Color;
    else
        diffuse = DiffuseColor;

    if (ColorMaterialMode == ECM_AMBIENT || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        ambient = input.Color;
    else
        ambient = AmbientColor;

    if (ColorMaterialMode == ECM_SPECULAR)
        specular = input.Color;
    else
        specular = SpecularColor;

    if (ColorMaterialMode == ECM_EMISSIVE)
        emissive = input.Color;
    else
        emissive = EmissiveColor;

    float3 nor = normalize(input.Normal);
    float3 lightDir1 = normalize(LightDirection);
    diffuse *= dot(nor, lightDir1);
    float4 finalColor = texColor * diffuse;
    //finalColor += ambient;
    //finalColor += emissive;

#if 0
    return float4(finalColor.rgb * input.Color.a, texColor.a * input.Color.a);
#else
    return finalColor;
#endif
}

# 81
Git commit: Add  camera pos to PS for calculating view direction by MiniMax-M2.7.
1. 在每次draw PS设置时，如果m_Material.Lighting开启，则获取当前camera position.
2. 为camera position创建camera constant buffer
3. 把camera constant buffer设置到PS slot PS_CAMERA_BUFFER_SLOT

CD3D11Driver对象不能获取到m_SceneManager因此下列函数没法计算出cameraPos,能够通过m_Matrices[ETS_VIEW]（View矩阵）计算出当前的视线向量，直接传给m_CameraConstantBuffer
        void CD3D11Driver::updateCameraConstantBuffer()
        {
            if (!m_SceneManager)
                return;

            irr::scene::ICameraSceneNode    *camera = m_SceneManager->getActiveCamera();
            if (!camera)
                return;

            core::vector3df    camPos = camera->getPosition();

            D3D11_MAPPED_SUBRESOURCE    mapped;
            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_CameraConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            {
                float    *data = (float*)mapped.pData;

                data[0] = camPos.X;
                data[1] = camPos.Y;
                data[2] = camPos.Z;

                m_pID3DDeviceContext->Unmap(m_CameraConstantBuffer, 0);

                ID3D11Buffer    *buffers[1] = {m_CameraConstantBuffer};
                m_pID3DDeviceContext->PSSetConstantBuffers(PS_CAMERA_BUFFER_SLOT, 1, buffers);
            }
        }

# 82
Git commit: 
把下面HLSL源代码定义迁移到CD3D11Shader.cpp文件中
        static const char    VERTEX_SHADER_STANDARD[] =
        static const char    VERTEX_SHADER_STANDARD_DIRECTIONAL[] =
        static const char    VERTEX_SHADER_STANDARD_POINT[] =
        static const char    VERTEX_SHADER_STANDARD_SPOT[] =
        static const char    VERTEX_SHADER_2TCOORDS_DIRECTIONAL[] =
        static const char    VERTEX_SHADER_2TCOORDS_POINT[] =
        static const char    VERTEX_SHADER_2TCOORDS_SPOT[] =
        static const char    VERTEX_SHADER_2TCOORDS[] =
        static const char    VERTEX_SHADER_RECTANGLE[] =
        static const char    PIXEL_SHADER_RECTANGLE[] =
        static const char    VERTEX_SHADER_TANGENTS[] =
        static const char    PIXEL_SHADER_STANDARD[] =
        static const char    PIXEL_SHADER_2TCOORDS[] =
        static const char    PIXEL_SHADER_TANGENTS[] =


# 83
Git commit: 


# 84
Git commit: 


# 85
Git commit: 


# 86
Git commit: 


# 87
Git commit: 
