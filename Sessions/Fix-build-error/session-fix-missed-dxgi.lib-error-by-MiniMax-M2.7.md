# 修复 CreateDXGIFactory1 链接错误

**Session ID:** ses_222a816a7ffeV3wFIEG1arso5O
**Created:** 2026/4/30 15:43:39
**Updated:** 2026/4/30 15:44:29

---

## User

修复下列错误

错误	LNK2019	无法解析的外部符号 CreateDXGIFactory1，函数 "public: bool __cdecl irr::video::CD3D11Driver::initDriver(struct HWND__ *,bool)" (?initDriver@CD3D11Driver@video@irr@@QEAA_NPEAUHWND__@@_N@Z) 中引用了该符号	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.obj	1		
错误	LNK1120	1 个无法解析的外部命令	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\bin\Win64-visualstudio\Irrlicht.dll	1 

---

## Assistant (Build · MiniMax-M2.7 · 6.2s)

_Thinking:_

用户遇到了链接错误，缺少 CreateDXGIFactory1 符号。这通常是因为需要链接 dxgi.lib 库。让我先查看 CD3D11Driver.cpp 文件和项目配置。

**Tool: grep**

**Input:**
```json
{
  "pattern": "CreateDXGIFactory1",
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht"
}
```

**Output:**
```
Found 1 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 172:             hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&DXGIFactory);

```

**Tool: glob**

**Input:**
```json
{
  "pattern": "**/*.vcxproj",
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht.2019.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\01.HelloWorld\HelloWorld_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\Demo\Demo_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\26.OcclusionQuery\OcclusionQuery_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\25.XmlHandling\XmlHandling_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\tools\MeshConverter\MeshConverter_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\tools\IrrFontTool\newFontTool\irrFontTool_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\tools\GUIEditor\GUI Editor_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\tools\MeshConverter\MeshConverter_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht10.0.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\tools\MeshConverter\MeshConverter_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\tools\IrrFontTool\newFontTool\irrFontTool_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\tools\IrrFontTool\newFontTool\irrFontTool_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\tools\GUIEditor\GUI Editor_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\tools\GUIEditor\GUI Editor_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\libpng\projects\vstudio\zlib\zlib.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\libpng\projects\vstudio\pngvalid\pngvalid.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\libpng\projects\vstudio\pngtest\pngtest.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\libpng\projects\vstudio\pnglibconf\pnglibconf.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\libpng\projects\vstudio\libpng\libpng.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht12.0.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht11.0.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\Demo\Demo_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\Demo\Demo_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\26.OcclusionQuery\OcclusionQuery_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\26.OcclusionQuery\OcclusionQuery_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\25.XmlHandling\XmlHandling_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\25.XmlHandling\XmlHandling_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\24.CursorControl\CursorControl_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\24.CursorControl\CursorControl_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\24.CursorControl\CursorControl_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\23.SMeshHandling\SMeshHandling_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\23.SMeshHandling\SMeshHandling_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\23.SMeshHandling\SMeshHandling_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\22.MaterialViewer\MaterialViewer_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\22.MaterialViewer\MaterialViewer_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\22.MaterialViewer\MaterialViewer_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\21.Quake3Explorer\Quake3Explorer_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\21.Quake3Explorer\Quake3Explorer_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\21.Quake3Explorer\Quake3Explorer_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\20.ManagedLights\ManagedLights_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\20.ManagedLights\ManagedLights_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\20.ManagedLights\ManagedLights_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\15.LoadIrrFile\LoadIrrFile_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\15.LoadIrrFile\LoadIrrFile_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\15.LoadIrrFile\LoadIrrFile_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\19.MouseAndJoystick\MouseAndJoystick_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\19.MouseAndJoystick\MouseAndJoystick_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\19.MouseAndJoystick\MouseAndJoystick_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\18.SplitScreen\SplitScreen_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\18.SplitScreen\SplitScreen_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\18.SplitScreen\SplitScreen_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\17.HelloWorld_Mobile\17. HelloWorld for Windows Mobile on PC_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\17.HelloWorld_Mobile\17. HelloWorld for Windows Mobile on PC_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\17.HelloWorld_Mobile\17. HelloWorld for Windows Mobile on PC_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\16.Quake3MapShader\Quake3MapShader_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\16.Quake3MapShader\Quake3MapShader_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\16.Quake3MapShader\Quake3MapShader_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\14.Win32Window\Win32Window_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\14.Win32Window\Win32Window_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\14.Win32Window\Win32Window_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\13.RenderToTexture\RenderToTexture_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\13.RenderToTexture\RenderToTexture_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\13.RenderToTexture\RenderToTexture_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\12.TerrainRendering\TerrainRendering_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\12.TerrainRendering\TerrainRendering_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\12.TerrainRendering\TerrainRendering_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\11.PerPixelLighting\PerPixelLighting_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\11.PerPixelLighting\PerPixelLighting_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\11.PerPixelLighting\PerPixelLighting_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\10.Shaders\Shaders_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\10.Shaders\Shaders_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\10.Shaders\Shaders_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\09.Meshviewer\Meshviewer_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\09.Meshviewer\Meshviewer_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\09.Meshviewer\Meshviewer_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\08.SpecialFX\SpecialFX_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\08.SpecialFX\SpecialFX_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\08.SpecialFX\SpecialFX_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\07.Collision\Collision_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\07.Collision\Collision_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\07.Collision\Collision_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\06.2DGraphics\2DGraphics_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\06.2DGraphics\2DGraphics_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\06.2DGraphics\2DGraphics_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\05.UserInterface\UserInterface_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\05.UserInterface\UserInterface_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\05.UserInterface\UserInterface_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\04.Movement\Movement_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\04.Movement\Movement_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\04.Movement\Movement_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\03.CustomSceneNode\CustomSceneNode_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\03.CustomSceneNode\CustomSceneNode_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\03.CustomSceneNode\CustomSceneNode_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\02.Quake3Map\Quake3Map_vc12.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\02.Quake3Map\Quake3Map_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\02.Quake3Map\Quake3Map_vc10.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\01.HelloWorld\HelloWorld_vc11.vcxproj
C:\Development\Graphics\irrlicht-1.8.3\examples\01.HelloWorld\HelloWorld_vc10.vcxproj
```

---

## Assistant (Build · MiniMax-M2.7 · 4.3s)

_Thinking:_

用户使用的是 Visual Studio 2019 ( Irrlicht.2019.vcxproj )。我需要查看这个项目的配置，看看是否已经添加了 dxgi.lib 库。CreateDXGIFactory1 是 DXGI (DirectX Graphics Infrastructure) 的一部分，需要链接 dxgi.lib。

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\Irrlicht.2019.vcxproj"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht.2019.vcxproj</path>
<type>file</type>
<content>
1: <?xml version="1.0" encoding="utf-8"?>
2: <Project DefaultTargets="Build" ToolsVersion="12.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
3:   <ItemGroup Label="ProjectConfigurations">
4:     <ProjectConfiguration Include="Debug|Win32">
5:       <Configuration>Debug</Configuration>
6:       <Platform>Win32</Platform>
7:     </ProjectConfiguration>
8:     <ProjectConfiguration Include="Debug|x64">
9:       <Configuration>Debug</Configuration>
10:       <Platform>x64</Platform>
11:     </ProjectConfiguration>
12:     <ProjectConfiguration Include="Release - Fast FPU|Win32">
13:       <Configuration>Release - Fast FPU</Configuration>
14:       <Platform>Win32</Platform>
15:     </ProjectConfiguration>
16:     <ProjectConfiguration Include="Release - Fast FPU|x64">
17:       <Configuration>Release - Fast FPU</Configuration>
18:       <Platform>x64</Platform>
19:     </ProjectConfiguration>
20:     <ProjectConfiguration Include="Release|Win32">
21:       <Configuration>Release</Configuration>
22:       <Platform>Win32</Platform>
23:     </ProjectConfiguration>
24:     <ProjectConfiguration Include="Release|x64">
25:       <Configuration>Release</Configuration>
26:       <Platform>x64</Platform>
27:     </ProjectConfiguration>
28:     <ProjectConfiguration Include="SDL-Debug|Win32">
29:       <Configuration>SDL-Debug</Configuration>
30:       <Platform>Win32</Platform>
31:     </ProjectConfiguration>
32:     <ProjectConfiguration Include="SDL-Debug|x64">
33:       <Configuration>SDL-Debug</Configuration>
34:       <Platform>x64</Platform>
35:     </ProjectConfiguration>
36:     <ProjectConfiguration Include="Static lib - Debug|Win32">
37:       <Configuration>Static lib - Debug</Configuration>
38:       <Platform>Win32</Platform>
39:     </ProjectConfiguration>
40:     <ProjectConfiguration Include="Static lib - Debug|x64">
41:       <Configuration>Static lib - Debug</Configuration>
42:       <Platform>x64</Platform>
43:     </ProjectConfiguration>
44:     <ProjectConfiguration Include="Static lib - Release - Fast FPU|Win32">
45:       <Configuration>Static lib - Release - Fast FPU</Configuration>
46:       <Platform>Win32</Platform>
47:     </ProjectConfiguration>
48:     <ProjectConfiguration Include="Static lib - Release - Fast FPU|x64">
49:       <Configuration>Static lib - Release - Fast FPU</Configuration>
50:       <Platform>x64</Platform>
51:     </ProjectConfiguration>
52:     <ProjectConfiguration Include="Static lib - Release|Win32">
53:       <Configuration>Static lib - Release</Configuration>
54:       <Platform>Win32</Platform>
55:     </ProjectConfiguration>
56:     <ProjectConfiguration Include="Static lib - Release|x64">
57:       <Configuration>Static lib - Release</Configuration>
58:       <Platform>x64</Platform>
59:     </ProjectConfiguration>
60:   </ItemGroup>
61:   <PropertyGroup Label="Globals">
62:     <ProjectName>Irrlicht</ProjectName>
63:     <ProjectGuid>{E08E042A-6C45-411B-92BE-3CC31331019F}</ProjectGuid>
64:     <RootNamespace>Irrlicht</RootNamespace>
65:     <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
66:   </PropertyGroup>
67:   <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
68:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'" Label="Configuration">
69:     <ConfigurationType>DynamicLibrary</ConfigurationType>
70:     <CharacterSet>NotSet</CharacterSet>
71:     <PlatformToolset>v143</PlatformToolset>
72:   </PropertyGroup>
73:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'" Label="Configuration">
74:     <ConfigurationType>DynamicLibrary</ConfigurationType>
75:     <CharacterSet>NotSet</CharacterSet>
76:     <PlatformToolset>v143</PlatformToolset>
77:   </PropertyGroup>
78:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'" Label="Configuration">
79:     <ConfigurationType>StaticLibrary</ConfigurationType>
80:     <CharacterSet>MultiByte</CharacterSet>
81:     <WholeProgramOptimization>true</WholeProgramOptimization>
82:     <PlatformToolset>v143</PlatformToolset>
83:   </PropertyGroup>
84:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'" Label="Configuration">
85:     <ConfigurationType>StaticLibrary</ConfigurationType>
86:     <CharacterSet>MultiByte</CharacterSet>
87:     <WholeProgramOptimization>true</WholeProgramOptimization>
88:     <PlatformToolset>v143</PlatformToolset>
89:   </PropertyGroup>
90:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'" Label="Configuration">
91:     <ConfigurationType>StaticLibrary</ConfigurationType>
92:     <CharacterSet>MultiByte</CharacterSet>
93:     <WholeProgramOptimization>true</WholeProgramOptimization>
94:     <PlatformToolset>v143</PlatformToolset>
95:   </PropertyGroup>
96:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'" Label="Configuration">
97:     <ConfigurationType>StaticLibrary</ConfigurationType>
98:     <CharacterSet>MultiByte</CharacterSet>
99:     <WholeProgramOptimization>true</WholeProgramOptimization>
100:     <PlatformToolset>v143</PlatformToolset>
101:   </PropertyGroup>
102:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'" Label="Configuration">
103:     <ConfigurationType>StaticLibrary</ConfigurationType>
104:     <CharacterSet>NotSet</CharacterSet>
105:     <PlatformToolset>v143</PlatformToolset>
106:   </PropertyGroup>
107:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'" Label="Configuration">
108:     <ConfigurationType>StaticLibrary</ConfigurationType>
109:     <CharacterSet>NotSet</CharacterSet>
110:     <PlatformToolset>v143</PlatformToolset>
111:   </PropertyGroup>
112:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'" Label="Configuration">
113:     <ConfigurationType>DynamicLibrary</ConfigurationType>
114:     <CharacterSet>MultiByte</CharacterSet>
115:     <WholeProgramOptimization>true</WholeProgramOptimization>
116:     <PlatformToolset>v143</PlatformToolset>
117:   </PropertyGroup>
118:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'" Label="Configuration">
119:     <ConfigurationType>DynamicLibrary</ConfigurationType>
120:     <CharacterSet>MultiByte</CharacterSet>
121:     <WholeProgramOptimization>true</WholeProgramOptimization>
122:     <PlatformToolset>v143</PlatformToolset>
123:   </PropertyGroup>
124:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
125:     <ConfigurationType>DynamicLibrary</ConfigurationType>
126:     <CharacterSet>MultiByte</CharacterSet>
127:     <WholeProgramOptimization>true</WholeProgramOptimization>
128:     <PlatformToolset>v143</PlatformToolset>
129:   </PropertyGroup>
130:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
131:     <ConfigurationType>DynamicLibrary</ConfigurationType>
132:     <CharacterSet>MultiByte</CharacterSet>
133:     <WholeProgramOptimization>true</WholeProgramOptimization>
134:     <PlatformToolset>v143</PlatformToolset>
135:   </PropertyGroup>
136:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
137:     <ConfigurationType>DynamicLibrary</ConfigurationType>
138:     <CharacterSet>NotSet</CharacterSet>
139:     <PlatformToolset>v143</PlatformToolset>
140:   </PropertyGroup>
141:   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
142:     <ConfigurationType>DynamicLibrary</ConfigurationType>
143:     <CharacterSet>NotSet</CharacterSet>
144:     <PlatformToolset>v143</PlatformToolset>
145:   </PropertyGroup>
146:   <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
147:   <ImportGroup Label="ExtensionSettings">
148:   </ImportGroup>
149:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'" Label="PropertySheets">
150:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
151:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
152:   </ImportGroup>
153:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'" Label="PropertySheets">
154:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
155:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
156:   </ImportGroup>
157:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'" Label="PropertySheets">
158:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
159:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
160:   </ImportGroup>
161:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'" Label="PropertySheets">
162:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
163:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
164:   </ImportGroup>
165:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'" Label="PropertySheets">
166:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
167:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
168:   </ImportGroup>
169:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'" Label="PropertySheets">
170:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
171:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
172:   </ImportGroup>
173:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'" Label="PropertySheets">
174:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
175:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
176:   </ImportGroup>
177:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'" Label="PropertySheets">
178:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
179:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
180:   </ImportGroup>
181:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'" Label="PropertySheets">
182:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
183:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
184:   </ImportGroup>
185:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'" Label="PropertySheets">
186:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
187:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
188:   </ImportGroup>
189:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="PropertySheets">
190:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
191:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
192:   </ImportGroup>
193:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="PropertySheets">
194:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
195:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
196:   </ImportGroup>
197:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="PropertySheets">
198:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
199:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
200:   </ImportGroup>
201:   <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="PropertySheets">
202:     <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
203:     <Import Project="$(VCTargetsPath)Microsoft.CPP.UpgradeFromVC71.props" />
204:   </ImportGroup>
205:   <PropertyGroup Label="UserMacros" />
206:   <PropertyGroup>
207:     <_ProjectFileVersion>10.0.30319.1</_ProjectFileVersion>
208:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">..\..\bin\Win32-VisualStudio\</OutDir>
209:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">..\..\bin\Win64-VisualStudio\</OutDir>
210:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">..\..\bin\Win32-VisualStudio\</OutDir>
211:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Release|x64'">..\..\bin\Win64-VisualStudio\</OutDir>
212:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">obj\$(Configuration)\</IntDir>
213:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Release|x64'">obj\$(Configuration)64\</IntDir>
214:     <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">false</LinkIncremental>
215:     <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Release|x64'">false</LinkIncremental>
216:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'">..\..\bin\Win32-VisualStudio\</OutDir>
217:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'">..\..\bin\Win64-VisualStudio\</OutDir>
218:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'">obj\$(Configuration)\</IntDir>
219:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'">obj\$(Configuration)64\</IntDir>
220:     <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'">false</LinkIncremental>
221:     <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'">false</LinkIncremental>
222:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'">..\..\lib\Win32-VisualStudio\</OutDir>
223:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'">..\..\lib\Win64-VisualStudio\</OutDir>
224:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'">obj\$(Configuration)\</IntDir>
225:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'">obj\$(Configuration)64\</IntDir>
226:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'">..\..\lib\Win32-VisualStudio\</OutDir>
227:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'">..\..\lib\Win64-VisualStudio\</OutDir>
228:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'">obj\$(Configuration)\</IntDir>
229:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'">obj\$(Configuration)64\</IntDir>
230:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'">..\..\lib\Win32-VisualStudio\</OutDir>
231:     <OutDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'">..\..\lib\Win64-VisualStudio\</OutDir>
232:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'">obj\$(Configuration)\</IntDir>
233:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'">obj\$(Configuration)64\</IntDir>
234:     <OutDir Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'">..\..\bin\Win32-VisualStudio\</OutDir>
235:     <OutDir Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'">..\..\bin\Win64-VisualStudio\</OutDir>
236:     <IntDir Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'">obj\$(Configuration)\</IntDir>
237:     <IntDir Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'">obj\$(Configuration)64\</IntDir>
238:     <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'">true</LinkIncremental>
239:     <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'">true</LinkIncremental>
240:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">AllRules.ruleset</CodeAnalysisRuleSet>
241:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">AllRules.ruleset</CodeAnalysisRuleSet>
242:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" />
243:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
244:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" />
245:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
246:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'">AllRules.ruleset</CodeAnalysisRuleSet>
247:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'">AllRules.ruleset</CodeAnalysisRuleSet>
248:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'" />
249:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'" />
250:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'" />
251:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'" />
252:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">AllRules.ruleset</CodeAnalysisRuleSet>
253:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Release|x64'">AllRules.ruleset</CodeAnalysisRuleSet>
254:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" />
255:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Release|x64'" />
256:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" />
257:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Release|x64'" />
258:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'">AllRules.ruleset</CodeAnalysisRuleSet>
259:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'">AllRules.ruleset</CodeAnalysisRuleSet>
260:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'" />
261:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'" />
262:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'" />
263:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'" />
264:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'">AllRules.ruleset</CodeAnalysisRuleSet>
265:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'">AllRules.ruleset</CodeAnalysisRuleSet>
266:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'" />
267:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'" />
268:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'" />
269:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'" />
270:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'">AllRules.ruleset</CodeAnalysisRuleSet>
271:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'">AllRules.ruleset</CodeAnalysisRuleSet>
272:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'" />
273:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'" />
274:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'" />
275:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'" />
276:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'">AllRules.ruleset</CodeAnalysisRuleSet>
277:     <CodeAnalysisRuleSet Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'">AllRules.ruleset</CodeAnalysisRuleSet>
278:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'" />
279:     <CodeAnalysisRules Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'" />
280:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'" />
281:     <CodeAnalysisRuleAssemblies Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'" />
282:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(CG_INC_PATH);$(IncludePath)</IncludePath>
283:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
284:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">obj\$(Configuration)\</IntDir>
285:     <IntDir Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">obj\$(Configuration)64\</IntDir>
286:     <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</LinkIncremental>
287:     <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">true</LinkIncremental>
288:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
289:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Release|x64'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
290:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
291:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
292:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
293:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
294:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
295:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
296:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
297:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
298:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
299:     <IncludePath Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'">C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;$(IncludePath)</IncludePath>
300:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">$(DXSDK_DIR)Lib\x86;$(CG_LIB_PATH);$(LibraryPath)</LibraryPath>
301:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'">$(DXSDK_DIR)Lib\x86;$(LibraryPath)</LibraryPath>
302:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">$(DXSDK_DIR)Lib\x86;$(LibraryPath)</LibraryPath>
303:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'">$(DXSDK_DIR)Lib\x86;$(LibraryPath)</LibraryPath>
304:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'">$(DXSDK_DIR)Lib\x86;$(LibraryPath)</LibraryPath>
305:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'">$(DXSDK_DIR)Lib\x86;$(LibraryPath)</LibraryPath>
306:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'">$(DXSDK_DIR)Lib\x86;$(LibraryPath)</LibraryPath>
307:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">$(DXSDK_DIR)Lib\x64;$(LibraryPath);$(VSInstallDir);$(VSInstallDir)lib\amd64</LibraryPath>
308:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'">$(DXSDK_DIR)Lib\x64;$(LibraryPath);$(VSInstallDir);$(VSInstallDir)lib\amd64</LibraryPath>
309:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Release|x64'">$(DXSDK_DIR)Lib\x64;$(LibraryPath);$(VSInstallDir);$(VSInstallDir)lib\amd64</LibraryPath>
310:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|x64'">$(DXSDK_DIR)Lib\x64;$(LibraryPath);$(VSInstallDir);$(VSInstallDir)lib\amd64</LibraryPath>
311:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'">$(DXSDK_DIR)Lib\x64;$(LibraryPath);$(VSInstallDir);$(VSInstallDir)lib\amd64</LibraryPath>
312:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'">$(DXSDK_DIR)Lib\x64;$(LibraryPath);$(VSInstallDir);$(VSInstallDir)lib\amd64</LibraryPath>
313:     <LibraryPath Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'">$(DXSDK_DIR)Lib\x64;$(LibraryPath);$(VSInstallDir);$(VSInstallDir)lib\amd64</LibraryPath>
314:   </PropertyGroup>
315:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
316:     <Midl>
317:       <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
318:       <MkTypLibCompatible>true</MkTypLibCompatible>
319:       <SuppressStartupBanner>true</SuppressStartupBanner>
320:       <TargetEnvironment>Win32</TargetEnvironment>
321:       <TypeLibraryName>.\..\Debug/Irrlicht.tlb</TypeLibraryName>
322:     </Midl>
323:     <ClCompile>
324:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
325:       <PreprocessorDefinitions>WIN32;_DEBUG;_WINDOWS;_USRDLL;IRRLICHT_EXPORTS;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions); _ITERATOR_DEBUG_LEVEL=0</PreprocessorDefinitions>
326:       <ExceptionHandling>
327:       </ExceptionHandling>
328:       <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
329:       <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
330:       <DisableLanguageExtensions>false</DisableLanguageExtensions>
331:       <RuntimeTypeInfo>true</RuntimeTypeInfo>
332:       <WarningLevel>Level3</WarningLevel>
333:       <Optimization>Disabled</Optimization>
334:       <DisableSpecificWarnings>26429;26493;4477;26481;26446;26482;26472;26440;26432;4267;6385;6386;6011;6326;26485;26433;26455;26434;26401;26402;26409;26400;26451;4996;26494;26495;26447;26496;26497;26467;26812;26430;26490</DisableSpecificWarnings>
335:       <BufferSecurityCheck>false</BufferSecurityCheck>
336:     </ClCompile>
337:     <ResourceCompile>
338:       <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
339:       <Culture>0x0c07</Culture>
340:     </ResourceCompile>
341:     <ProjectReference>
342:       <UseLibraryDependencyInputs>true</UseLibraryDependencyInputs>
343:     </ProjectReference>
344:     <Link>
345:       <AdditionalOptions>/MACHINE:I386 %(AdditionalOptions)</AdditionalOptions>
346:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;%(AdditionalDependencies)</AdditionalDependencies>
347:       <OutputFile>..\..\bin\Win32-visualstudio\Irrlicht.dll</OutputFile>
348:       <AdditionalLibraryDirectories>%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
349:       <IgnoreSpecificDefaultLibraries>libci.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
350:       <GenerateDebugInformation>true</GenerateDebugInformation>
351:       <DataExecutionPrevention>
352:       </DataExecutionPrevention>
353:       <ImportLibrary>..\..\lib\Win32-visualstudio\Irrlicht.lib</ImportLibrary>
354:       <Version>1.8</Version>
355:     </Link>
356:   </ItemDefinitionGroup>
357:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
358:     <Midl>
359:       <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
360:       <MkTypLibCompatible>true</MkTypLibCompatible>
361:       <SuppressStartupBanner>true</SuppressStartupBanner>
362:       <TypeLibraryName>.\..\Debug/Irrlicht.tlb</TypeLibraryName>
363:     </Midl>
364:     <ClCompile>
365:       <AdditionalIncludeDirectories>C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.26100.0;..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
366:       <PreprocessorDefinitions>WIN32;WIN64;_DEBUG;_WINDOWS;_USRDLL;IRRLICHT_EXPORTS;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions); _ITERATOR_DEBUG_LEVEL=0</PreprocessorDefinitions>
367:       <ExceptionHandling>
368:       </ExceptionHandling>
369:       <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
370:       <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
371:       <DisableLanguageExtensions>false</DisableLanguageExtensions>
372:       <RuntimeTypeInfo>true</RuntimeTypeInfo>
373:       <WarningLevel>Level3</WarningLevel>
374:       <Optimization>Disabled</Optimization>
375:       <DisableSpecificWarnings>26429;26493;4477;26481;26446;26482;26472;26440;26432;4267;6385;6386;6011;6326;26485;26433;26455;26434;26401;26402;26409;26400;26451;4996;26494;26495;26447;26496;26497;26467;26812;26430;26490;4013</DisableSpecificWarnings>
376:       <BufferSecurityCheck>false</BufferSecurityCheck>
377:     </ClCompile>
378:     <ResourceCompile>
379:       <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
380:       <Culture>0x0c07</Culture>
381:     </ResourceCompile>
382:     <ProjectReference>
383:       <UseLibraryDependencyInputs>true</UseLibraryDependencyInputs>
384:     </ProjectReference>
385:     <Link>
386:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
387:       <OutputFile>..\..\bin\Win64-visualstudio\Irrlicht.dll</OutputFile>
388:       <AdditionalLibraryDirectories>%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
389:       <IgnoreSpecificDefaultLibraries>libci.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
390:       <GenerateDebugInformation>true</GenerateDebugInformation>
391:       <DataExecutionPrevention>
392:       </DataExecutionPrevention>
393:       <ImportLibrary>..\..\lib\Win64-visualstudio\Irrlicht.lib</ImportLibrary>
394:     </Link>
395:   </ItemDefinitionGroup>
396:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
397:     <Midl>
398:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
399:       <MkTypLibCompatible>true</MkTypLibCompatible>
400:       <SuppressStartupBanner>true</SuppressStartupBanner>
401:       <TargetEnvironment>Win32</TargetEnvironment>
402:       <TypeLibraryName>.\..\Release/Irrlicht.tlb</TypeLibraryName>
403:     </Midl>
404:     <ClCompile>
405:       <Optimization>MaxSpeed</Optimization>
406:       <InlineFunctionExpansion>OnlyExplicitInline</InlineFunctionExpansion>
407:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
408:       <PreprocessorDefinitions>WIN32;NDEBUG;_WINDOWS;_USRDLL;IRRLICHT_EXPORTS;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
409:       <StringPooling>true</StringPooling>
410:       <ExceptionHandling>
411:       </ExceptionHandling>
412:       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
413:       <BufferSecurityCheck>false</BufferSecurityCheck>
414:       <FunctionLevelLinking>true</FunctionLevelLinking>
415:       <WarningLevel>Level3</WarningLevel>
416:       <DebugInformationFormat>
417:       </DebugInformationFormat>
418:       <CompileAs>Default</CompileAs>
419:     </ClCompile>
420:     <ResourceCompile>
421:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
422:       <Culture>0x0c07</Culture>
423:     </ResourceCompile>
424:     <Link>
425:       <AdditionalOptions>/MACHINE:I386 %(AdditionalOptions)</AdditionalOptions>
426:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;opengl32.lib;%(AdditionalDependencies)</AdditionalDependencies>
427:       <OutputFile>..\..\bin\Win32-visualstudio\Irrlicht.dll</OutputFile>
428:       <IgnoreSpecificDefaultLibraries>libci.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
429:       <GenerateDebugInformation>false</GenerateDebugInformation>
430:       <DataExecutionPrevention>
431:       </DataExecutionPrevention>
432:       <ImportLibrary>..\..\lib\Win32-visualstudio\Irrlicht.lib</ImportLibrary>
433:       <Version>1.8</Version>
434:     </Link>
435:   </ItemDefinitionGroup>
436:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
437:     <Midl>
438:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
439:       <MkTypLibCompatible>true</MkTypLibCompatible>
440:       <SuppressStartupBanner>true</SuppressStartupBanner>
441:       <TypeLibraryName>.\..\Release/Irrlicht.tlb</TypeLibraryName>
442:     </Midl>
443:     <ClCompile>
444:       <Optimization>MaxSpeed</Optimization>
445:       <InlineFunctionExpansion>OnlyExplicitInline</InlineFunctionExpansion>
446:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
447:       <PreprocessorDefinitions>WIN32;WIN64;NDEBUG;_WINDOWS;_USRDLL;IRRLICHT_EXPORTS;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
448:       <StringPooling>true</StringPooling>
449:       <ExceptionHandling>
450:       </ExceptionHandling>
451:       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
452:       <BufferSecurityCheck>false</BufferSecurityCheck>
453:       <FunctionLevelLinking>true</FunctionLevelLinking>
454:       <WarningLevel>Level3</WarningLevel>
455:       <DebugInformationFormat>
456:       </DebugInformationFormat>
457:       <CompileAs>Default</CompileAs>
458:     </ClCompile>
459:     <ResourceCompile>
460:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
461:       <Culture>0x0c07</Culture>
462:     </ResourceCompile>
463:     <Link>
464:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
465:       <OutputFile>..\..\bin\Win64-visualstudio\Irrlicht.dll</OutputFile>
466:       <IgnoreSpecificDefaultLibraries>libci.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
467:       <GenerateDebugInformation>false</GenerateDebugInformation>
468:       <DataExecutionPrevention>
469:       </DataExecutionPrevention>
470:       <ImportLibrary>..\..\lib\Win64-visualstudio\Irrlicht.lib</ImportLibrary>
471:     </Link>
472:   </ItemDefinitionGroup>
473:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|Win32'">
474:     <Midl>
475:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
476:       <MkTypLibCompatible>true</MkTypLibCompatible>
477:       <SuppressStartupBanner>true</SuppressStartupBanner>
478:       <TargetEnvironment>Win32</TargetEnvironment>
479:       <TypeLibraryName>.\..\Release/Irrlicht.tlb</TypeLibraryName>
480:     </Midl>
481:     <ClCompile>
482:       <Optimization>Full</Optimization>
483:       <InlineFunctionExpansion>AnySuitable</InlineFunctionExpansion>
484:       <IntrinsicFunctions>true</IntrinsicFunctions>
485:       <FavorSizeOrSpeed>Speed</FavorSizeOrSpeed>
486:       <OmitFramePointers>true</OmitFramePointers>
487:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
488:       <PreprocessorDefinitions>WIN32;NDEBUG;_WINDOWS;_USRDLL;IRRLICHT_EXPORTS;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
489:       <StringPooling>true</StringPooling>
490:       <ExceptionHandling>false</ExceptionHandling>
491:       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
492:       <StructMemberAlignment>Default</StructMemberAlignment>
493:       <BufferSecurityCheck>false</BufferSecurityCheck>
494:       <FunctionLevelLinking>false</FunctionLevelLinking>
495:       <EnableEnhancedInstructionSet>StreamingSIMDExtensions2</EnableEnhancedInstructionSet>
496:       <FloatingPointModel>Fast</FloatingPointModel>
497:       <RuntimeTypeInfo>false</RuntimeTypeInfo>
498:       <WarningLevel>Level3</WarningLevel>
499:       <DebugInformationFormat>
500:       </DebugInformationFormat>
501:     </ClCompile>
502:     <ResourceCompile>
503:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
504:       <Culture>0x0c07</Culture>
505:     </ResourceCompile>
506:     <Link>
507:       <AdditionalOptions>/MACHINE:I386 %(AdditionalOptions)</AdditionalOptions>
508:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;%(AdditionalDependencies)</AdditionalDependencies>
509:       <OutputFile>..\..\bin\Win32-visualstudio\Irrlicht.dll</OutputFile>
510:       <IgnoreSpecificDefaultLibraries>libci.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
511:       <GenerateDebugInformation>false</GenerateDebugInformation>
512:       <OptimizeReferences>true</OptimizeReferences>
513:       <DataExecutionPrevention>
514:       </DataExecutionPrevention>
515:       <ImportLibrary>..\..\lib\Win32-visualstudio\Irrlicht.lib</ImportLibrary>
516:       <SubSystem>Windows</SubSystem>
517:       <Version>1.8</Version>
518:     </Link>
519:   </ItemDefinitionGroup>
520:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release - Fast FPU|x64'">
521:     <Midl>
522:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
523:       <MkTypLibCompatible>true</MkTypLibCompatible>
524:       <SuppressStartupBanner>true</SuppressStartupBanner>
525:       <TypeLibraryName>.\..\Release/Irrlicht.tlb</TypeLibraryName>
526:     </Midl>
527:     <ClCompile>
528:       <Optimization>Full</Optimization>
529:       <InlineFunctionExpansion>AnySuitable</InlineFunctionExpansion>
530:       <IntrinsicFunctions>true</IntrinsicFunctions>
531:       <FavorSizeOrSpeed>Speed</FavorSizeOrSpeed>
532:       <OmitFramePointers>true</OmitFramePointers>
533:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
534:       <PreprocessorDefinitions>WIN32;WIN64;NDEBUG;_WINDOWS;_USRDLL;IRRLICHT_EXPORTS;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
535:       <StringPooling>true</StringPooling>
536:       <ExceptionHandling>false</ExceptionHandling>
537:       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
538:       <StructMemberAlignment>Default</StructMemberAlignment>
539:       <BufferSecurityCheck>false</BufferSecurityCheck>
540:       <FunctionLevelLinking>false</FunctionLevelLinking>
541:       <EnableEnhancedInstructionSet>StreamingSIMDExtensions2</EnableEnhancedInstructionSet>
542:       <FloatingPointModel>Fast</FloatingPointModel>
543:       <RuntimeTypeInfo>false</RuntimeTypeInfo>
544:       <WarningLevel>Level3</WarningLevel>
545:       <DebugInformationFormat>
546:       </DebugInformationFormat>
547:     </ClCompile>
548:     <ResourceCompile>
549:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
550:       <Culture>0x0c07</Culture>
551:     </ResourceCompile>
552:     <Link>
553:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
554:       <OutputFile>..\..\bin\Win64-visualstudio\Irrlicht.dll</OutputFile>
555:       <IgnoreSpecificDefaultLibraries>libci.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
556:       <GenerateDebugInformation>false</GenerateDebugInformation>
557:       <OptimizeReferences>true</OptimizeReferences>
558:       <DataExecutionPrevention>
559:       </DataExecutionPrevention>
560:       <ImportLibrary>..\..\lib\Win64-visualstudio\Irrlicht.lib</ImportLibrary>
561:       <SubSystem>Windows</SubSystem>
562:     </Link>
563:   </ItemDefinitionGroup>
564:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|Win32'">
565:     <Midl>
566:       <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
567:       <MkTypLibCompatible>true</MkTypLibCompatible>
568:       <SuppressStartupBanner>true</SuppressStartupBanner>
569:       <TargetEnvironment>Win32</TargetEnvironment>
570:       <TypeLibraryName>.\..\Debug/Irrlicht.tlb</TypeLibraryName>
571:     </Midl>
572:     <ClCompile>
573:       <Optimization>Disabled</Optimization>
574:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
575:       <PreprocessorDefinitions>WIN32;_DEBUG;_WINDOWS;_USRDLL;_IRR_STATIC_LIB_;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions); _ITERATOR_DEBUG_LEVEL=0</PreprocessorDefinitions>
576:       <ExceptionHandling>
577:       </ExceptionHandling>
578:       <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
579:       <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
580:       <DisableLanguageExtensions>false</DisableLanguageExtensions>
581:       <RuntimeTypeInfo>false</RuntimeTypeInfo>
582:       <WarningLevel>Level3</WarningLevel>
583:     </ClCompile>
584:     <ResourceCompile>
585:       <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
586:       <Culture>0x0c07</Culture>
587:     </ResourceCompile>
588:     <Lib>
589:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
590:       <OutputFile>..\..\lib\Win32-visualstudio\Irrlicht.lib</OutputFile>
591:     </Lib>
592:   </ItemDefinitionGroup>
593:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Debug|x64'">
594:     <Midl>
595:       <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
596:       <MkTypLibCompatible>true</MkTypLibCompatible>
597:       <SuppressStartupBanner>true</SuppressStartupBanner>
598:       <TypeLibraryName>.\..\Debug/Irrlicht.tlb</TypeLibraryName>
599:     </Midl>
600:     <ClCompile>
601:       <Optimization>Disabled</Optimization>
602:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
603:       <PreprocessorDefinitions>WIN32;WIN64;_DEBUG;_WINDOWS;_USRDLL;_IRR_STATIC_LIB_;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions); _ITERATOR_DEBUG_LEVEL=0</PreprocessorDefinitions>
604:       <ExceptionHandling>
605:       </ExceptionHandling>
606:       <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
607:       <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
608:       <DisableLanguageExtensions>false</DisableLanguageExtensions>
609:       <RuntimeTypeInfo>false</RuntimeTypeInfo>
610:       <WarningLevel>Level3</WarningLevel>
611:     </ClCompile>
612:     <ResourceCompile>
613:       <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
614:       <Culture>0x0c07</Culture>
615:     </ResourceCompile>
616:     <Lib>
617:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
618:       <OutputFile>..\..\lib\Win64-visualstudio\Irrlicht.lib</OutputFile>
619:       <TargetMachine>MachineX64</TargetMachine>
620:     </Lib>
621:   </ItemDefinitionGroup>
622:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|Win32'">
623:     <Midl>
624:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
625:       <MkTypLibCompatible>true</MkTypLibCompatible>
626:       <SuppressStartupBanner>true</SuppressStartupBanner>
627:       <TargetEnvironment>Win32</TargetEnvironment>
628:       <TypeLibraryName>.\..\Release/Irrlicht.tlb</TypeLibraryName>
629:     </Midl>
630:     <ClCompile>
631:       <Optimization>MaxSpeed</Optimization>
632:       <InlineFunctionExpansion>OnlyExplicitInline</InlineFunctionExpansion>
633:       <IntrinsicFunctions>false</IntrinsicFunctions>
634:       <WholeProgramOptimization>false</WholeProgramOptimization>
635:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
636:       <PreprocessorDefinitions>WIN32;NDEBUG;_WINDOWS;_USRDLL;_IRR_STATIC_LIB_;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
637:       <StringPooling>true</StringPooling>
638:       <ExceptionHandling>
639:       </ExceptionHandling>
640:       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
641:       <BufferSecurityCheck>false</BufferSecurityCheck>
642:       <FunctionLevelLinking>true</FunctionLevelLinking>
643:       <WarningLevel>Level3</WarningLevel>
644:       <DebugInformationFormat>
645:       </DebugInformationFormat>
646:     </ClCompile>
647:     <ResourceCompile>
648:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
649:       <Culture>0x0c07</Culture>
650:     </ResourceCompile>
651:     <Lib>
652:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
653:       <OutputFile>..\..\lib\Win32-visualstudio\Irrlicht.lib</OutputFile>
654:     </Lib>
655:   </ItemDefinitionGroup>
656:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release|x64'">
657:     <Midl>
658:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
659:       <MkTypLibCompatible>true</MkTypLibCompatible>
660:       <SuppressStartupBanner>true</SuppressStartupBanner>
661:       <TypeLibraryName>.\..\Release/Irrlicht.tlb</TypeLibraryName>
662:     </Midl>
663:     <ClCompile>
664:       <Optimization>MaxSpeed</Optimization>
665:       <InlineFunctionExpansion>OnlyExplicitInline</InlineFunctionExpansion>
666:       <IntrinsicFunctions>false</IntrinsicFunctions>
667:       <WholeProgramOptimization>false</WholeProgramOptimization>
668:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
669:       <PreprocessorDefinitions>WIN32;WIN64;NDEBUG;_WINDOWS;_USRDLL;_IRR_STATIC_LIB_;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
670:       <StringPooling>true</StringPooling>
671:       <ExceptionHandling>
672:       </ExceptionHandling>
673:       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
674:       <BufferSecurityCheck>false</BufferSecurityCheck>
675:       <FunctionLevelLinking>true</FunctionLevelLinking>
676:       <WarningLevel>Level3</WarningLevel>
677:       <DebugInformationFormat>
678:       </DebugInformationFormat>
679:     </ClCompile>
680:     <ResourceCompile>
681:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
682:       <Culture>0x0c07</Culture>
683:     </ResourceCompile>
684:     <Lib>
685:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
686:       <OutputFile>..\..\lib\Win64-visualstudio\Irrlicht.lib</OutputFile>
687:       <TargetMachine>MachineX64</TargetMachine>
688:     </Lib>
689:   </ItemDefinitionGroup>
690:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|Win32'">
691:     <Midl>
692:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
693:       <MkTypLibCompatible>true</MkTypLibCompatible>
694:       <SuppressStartupBanner>true</SuppressStartupBanner>
695:       <TargetEnvironment>Win32</TargetEnvironment>
696:       <TypeLibraryName>.\..\Release/Irrlicht.tlb</TypeLibraryName>
697:     </Midl>
698:     <ClCompile>
699:       <Optimization>Full</Optimization>
700:       <InlineFunctionExpansion>AnySuitable</InlineFunctionExpansion>
701:       <IntrinsicFunctions>true</IntrinsicFunctions>
702:       <FavorSizeOrSpeed>Speed</FavorSizeOrSpeed>
703:       <OmitFramePointers>true</OmitFramePointers>
704:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
705:       <PreprocessorDefinitions>WIN32;NDEBUG;_WINDOWS;_USRDLL;_IRR_STATIC_LIB_;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
706:       <StringPooling>true</StringPooling>
707:       <ExceptionHandling>
708:       </ExceptionHandling>
709:       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
710:       <BufferSecurityCheck>false</BufferSecurityCheck>
711:       <FunctionLevelLinking>true</FunctionLevelLinking>
712:       <FloatingPointModel>Fast</FloatingPointModel>
713:       <RuntimeTypeInfo>false</RuntimeTypeInfo>
714:       <WarningLevel>Level3</WarningLevel>
715:       <DebugInformationFormat>
716:       </DebugInformationFormat>
717:       <CallingConvention>FastCall</CallingConvention>
718:     </ClCompile>
719:     <ResourceCompile>
720:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
721:       <Culture>0x0c07</Culture>
722:     </ResourceCompile>
723:     <Lib>
724:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
725:       <OutputFile>..\..\lib\Win32-visualstudio\Irrlicht.lib</OutputFile>
726:       <TargetMachine>MachineX86</TargetMachine>
727:     </Lib>
728:   </ItemDefinitionGroup>
729:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Static lib - Release - Fast FPU|x64'">
730:     <Midl>
731:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
732:       <MkTypLibCompatible>true</MkTypLibCompatible>
733:       <SuppressStartupBanner>true</SuppressStartupBanner>
734:       <TypeLibraryName>.\..\Release/Irrlicht.tlb</TypeLibraryName>
735:     </Midl>
736:     <ClCompile>
737:       <Optimization>Full</Optimization>
738:       <InlineFunctionExpansion>AnySuitable</InlineFunctionExpansion>
739:       <IntrinsicFunctions>true</IntrinsicFunctions>
740:       <FavorSizeOrSpeed>Speed</FavorSizeOrSpeed>
741:       <OmitFramePointers>true</OmitFramePointers>
742:       <AdditionalIncludeDirectories>..\..\include;zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
743:       <PreprocessorDefinitions>WIN32;WIN64;NDEBUG;_WINDOWS;_USRDLL;_IRR_STATIC_LIB_;_CRT_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
744:       <StringPooling>true</StringPooling>
745:       <ExceptionHandling>
746:       </ExceptionHandling>
747:       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
748:       <BufferSecurityCheck>false</BufferSecurityCheck>
749:       <FunctionLevelLinking>true</FunctionLevelLinking>
750:       <FloatingPointModel>Fast</FloatingPointModel>
751:       <RuntimeTypeInfo>false</RuntimeTypeInfo>
752:       <WarningLevel>Level3</WarningLevel>
753:       <DebugInformationFormat>
754:       </DebugInformationFormat>
755:       <CallingConvention>FastCall</CallingConvention>
756:     </ClCompile>
757:     <ResourceCompile>
758:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
759:       <Culture>0x0c07</Culture>
760:     </ResourceCompile>
761:     <Lib>
762:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
763:       <OutputFile>..\..\lib\Win64-visualstudio\Irrlicht.lib</OutputFile>
764:       <TargetMachine>MachineX64</TargetMachine>
765:     </Lib>
766:   </ItemDefinitionGroup>
767:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='SDL-Debug|Win32'">
768:     <Midl>
769:       <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
770:       <MkTypLibCompatible>true</MkTypLibCompatible>
771:       <SuppressStartupBanner>true</SuppressStartupBanner>
772:       <TargetEnvironment>Win32</TargetEnvironment>
773:       <TypeLibraryName>.\..\Debug/Irrlicht.tlb</TypeLibraryName>

(Output capped at 50 KB. Showing lines 1-773. Use offset=774 to continue.)
</content>

<system-reminder>
Instructions from: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\AGENTS.md
# SOURCE KNOWLEDGE BASE

## OVERVIEW
Implementation of the Irrlicht Engine including rendering drivers, scene management, and 3rd party libraries.

## STRUCTURE
```
../include         # header files, including interface definition.
source/Irrlicht/
├── aesGladman/    # AES encryption
├── bzip2/         # bzip2 compression
├── jpeglib/       # JPEG image support
├── libpng/        # PNG image support
├── lzma/          # LZMA compression
├── MacOSX/        # macOS platform specifics (Cocoa/OpenGL)
└── zlib/          # zlib compression
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Rendering | `C*Driver.cpp` | `CNullDriver`, `COpenGLDriver`, `CD3D9Driver` |
| Scene Graph | `C*SceneNode.cpp` | `CSceneManager`, `CMeshSceneNode`, etc. |
| GUI | `CGUI*.cpp` | `CGUIEnvironment`, `CGUIButton`, etc. |
| IO | `C*Reader.cpp` | File system and archive handling |

## CONVENTIONS
- Internal classes prefixed with `C` (e.g., `CSceneManager` implements `ISceneManager`).
- Heavy use of `irr::core` containers for internal logic.
- Platform-specific code isolated in `CIrrDevice*.cpp` and OS subdirectories.

## ANTI-PATTERNS
- Do not add dependencies on `std::`.
- Avoid direct OS API calls outside of `CIrrDevice` or platform directories.

</system-reminder>
```

---

## Assistant (Build · MiniMax-M2.7 · 11.0s)

_Thinking:_

我看到问题了。项目使用的是 `Debug|x64` 配置，我可以看到在第 386 行，`AdditionalDependencies` 没有包含 dxgi.lib。`CreateDXGIFactory1` 是 DXGI 函数，需要链接 `dxgi.lib`。

我需要添加 `dxgi.lib` 到 `AdditionalDependencies`。我可以看到项目中没有链接 dxgi.lib。让我检查一下 Release|x64 配置（第 464 行）以及 SDL-Debug|x64 等配置。

问题是 CD3D11Driver.cpp 使用了 CreateDXGIFactory1，但项目没有链接 dxgi.lib。

需要修改 Irrlicht.2019.vcxproj 文件，在相关的 x64 配置中添加 dxgi.lib。

用户使用的是 Debug|x64 配置（从错误路径可以看出是 Win64-visualstudio）。

让我给 Debug|x64 配置添加 dxgi.lib：
第 386 行的 Debug|x64 AdditionalDependencies 需要添加 dxgi.lib

其他配置如 Release|x64、Release - Fast FPU|x64、SDL-Debug|x64 等也需要添加。

让我找到需要修改的地方并添加 dxgi.lib。

`CreateDXGIFactory1` 来自 DXGI，需要链接 `dxgi.lib`。项目配置中缺少此库。


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\Irrlicht.2019.vcxproj",
  "oldString": "<AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>\n      <OutputFile>..\\..\\bin\\Win64-visualstudio\\Irrlicht.dll</OutputFile>",
  "newString": "<AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;dxgi.lib;%(AdditionalDependencies)</AdditionalDependencies>\n      <OutputFile>..\\..\\bin\\Win64-visualstudio\\Irrlicht.dll</OutputFile>"
}
```

**Error:**
```
Found multiple matches for oldString. Provide more surrounding context to make the match unique.
```

---

## Assistant (Build · MiniMax-M2.7 · 5.1s)

_Thinking:_

有多个匹配，需要提供更多上下文。我看到有 Release - Fast FPU|x64 在第 553 行，Debug|x64 在第 386 行，SDL-Debug|x64 也可能有。让我读取引擎使用这个函数的配置，并提供更多上下文。

用户正在构建 Debug|x64 配置，所以我需要修改那一行。让我查看更多上下文。

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\Irrlicht.2019.vcxproj",
  "offset": 380,
  "limit": 20
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht.2019.vcxproj</path>
<type>file</type>
<content>
380:       <Culture>0x0c07</Culture>
381:     </ResourceCompile>
382:     <ProjectReference>
383:       <UseLibraryDependencyInputs>true</UseLibraryDependencyInputs>
384:     </ProjectReference>
385:     <Link>
386:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
387:       <OutputFile>..\..\bin\Win64-visualstudio\Irrlicht.dll</OutputFile>
388:       <AdditionalLibraryDirectories>%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
389:       <IgnoreSpecificDefaultLibraries>libci.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
390:       <GenerateDebugInformation>true</GenerateDebugInformation>
391:       <DataExecutionPrevention>
392:       </DataExecutionPrevention>
393:       <ImportLibrary>..\..\lib\Win64-visualstudio\Irrlicht.lib</ImportLibrary>
394:     </Link>
395:   </ItemDefinitionGroup>
396:   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
397:     <Midl>
398:       <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
399:       <MkTypLibCompatible>true</MkTypLibCompatible>

(Showing lines 380-399 of 1573. Use offset=400 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 9.5s)

_Thinking:_

我看到 Debug|x64 的配置从第 357 行开始，包含 395 行结束。这个配置中的 Link 部分在 385-394 行。我需要修改第 386 行的 AdditionalDependencies 来包含 dxgi.lib。

让我提供更多上下文来使其唯一。

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\Irrlicht.2019.vcxproj",
  "oldString": "<Link>\n      <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>\n      <OutputFile>..\\..\\bin\\Win64-visualstudio\\Irrlicht.dll</OutputFile>\n      <AdditionalLibraryDirectories>%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>\n      <IgnoreSpecificDefaultLibraries>libci.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n      <GenerateDebugInformation>true</GenerateDebugInformation>\n      <DataExecutionPrevention>\n      </DataExecutionPrevention>\n      <ImportLibrary>..\\..\\lib\\Win64-visualstudio\\Irrlicht.lib</ImportLibrary>\n    </Link>\n  </ItemDefinitionGroup>\n  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">",
  "newString": "<Link>\n      <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;dxgi.lib;%(AdditionalDependencies)</AdditionalDependencies>\n      <OutputFile>..\\..\\bin\\Win64-visualstudio\\Irrlicht.dll</OutputFile>\n      <AdditionalLibraryDirectories>%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>\n      <IgnoreSpecificDefaultLibraries>libci.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n      <GenerateDebugInformation>true</GenerateDebugInformation>\n      <DataExecutionPrevention>\n      </DataExecutionPrevention>\n      <ImportLibrary>..\\..\\lib\\Win64-visualstudio\\Irrlicht.lib</ImportLibrary>\n    </Link>\n  </ItemDefinitionGroup>\n  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 4.5s)

_Thinking:_

已经修复了 Debug|x64 配置。但我应该检查其他 x64 配置是否也缺少 dxgi.lib，因为 CD3D11Driver 可能被多个配置使用。让我检查 Release|x64、Release - Fast FPU|x64 等配置。


还需要为其他 x64 配置添加 `dxgi.lib`。让我检查并修复：


**Tool: grep**

**Input:**
```json
{
  "pattern": "AdditionalDependencies>.*winmm\\.lib",
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht"
}
```

**Output:**
```
Found 40 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht.2019.vcxproj:
  Line 386:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;dxgi.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 464:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 553:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 589:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 617:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 652:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 685:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 724:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 762:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 834:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht10.0.vcxproj:
  Line 382:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 460:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 549:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 585:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 613:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 648:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 681:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 720:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 758:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 830:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht12.0.vcxproj:
  Line 382:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 460:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 549:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 585:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 613:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 648:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 681:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 720:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 758:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 830:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht11.0.vcxproj:
  Line 383:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 461:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 550:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 586:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 614:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 649:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 682:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 721:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 759:       <AdditionalDependencies>winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

  Line 831:       <AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;opengl32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>

```

---

## Assistant (Build · MiniMax-M2.7 · 8.7s)

_Thinking:_

用户使用的是 Irrlicht.2019.vcxproj 文件，该文件已经被修改了 Debug|x64 配置。现在我需要检查 Release|x64 (第 464 行) 和 Release - Fast FPU|x64 (第 553 行) 以及 SDL-Debug|x64 (第 834 行) 是否也需要添加 dxgi.lib。

由于用户只报告了 Debug|x64 配置的问题，可能他们只需要修复那个配置。但是为了一致性和避免将来出现问题，我应该检查并修复所有相关的 x64 配置，这些配置可能会使用 CD3D11Driver。

但实际上，我应该只修复用户正在使用的配置。用户报告的错误是 Debug|x64 配置，我已经修复了。如果用户需要修复其他配置，他们会询问。

让我验证修改是否正确。

已在 `Debug|x64` 配置中添加 `dxgi.lib`。重新构建项目即可解决链接错误。

如果其他配置（如 Release|x64、SDL-Debug|x64 等）也需要支持 D3D11，应同样添加 `dxgi.lib`。

---

