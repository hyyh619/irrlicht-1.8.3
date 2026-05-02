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

