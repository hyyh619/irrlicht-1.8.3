@echo off
REM Build script for Irrlicht Engine using Visual Studio 2022
REM Platform: x64, Configuration: Debug, CONFORM_TEST enabled
REM
REM Usage: build_debug_x64_conform.bat [DX9|DX11]
REM   DX9  - Build with CONFORM_TEST_DX9 (Direct3D 9) - default
REM   DX11 - Build with CONFORM_TEST_DX11 (Direct3D 11)

set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
set "MSBUILD=%VS_PATH%\MSBuild\Current\Bin\MSBuild.exe"

if not exist "%MSBUILD%" (
    echo Error: MSBuild.exe not found at %MSBUILD%
    exit /b 1
)

set "VARIANT=%1"
if "%VARIANT%"=="" set "VARIANT=DX9"

if /i "%VARIANT%"=="DX9" (
    echo Building Irrlicht Engine (x64 Debug, CONFORM_TEST_DX9)...
    echo.
    copy Directory_DX9.Build.props Directory.Build.props
) else if /i "%VARIANT%"=="DX11" (
    echo Building Irrlicht Engine (x64 Debug, CONFORM_TEST_DX11)...
    echo.
    copy Directory_DX11.Build.props Directory.Build.props
) else (
    echo Error: Invalid variant "%VARIANT%"
    echo Usage: build_debug_x64_conform.bat [DX9^|DX11]
    exit /b 1
)

"%MSBUILD%" BuildAllExamples.2019.sln /p:Configuration=Debug /p:Platform=x64 /v:minimal
del Directory.Build.props

if %ERRORLEVEL% neq 0 (
    echo.
    echo Build FAILED!
    exit /b 1
)

echo.
echo Build completed successfully!
exit /b 0