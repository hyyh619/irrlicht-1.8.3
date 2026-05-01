@echo off
REM Build script for Irrlicht Engine using Visual Studio 2022
REM Platform: x64, Configuration: Debug, CONFORM_TEST_DX9 enabled

set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
set "MSBUILD=%VS_PATH%\MSBuild\Current\Bin\MSBuild.exe"

if not exist "%MSBUILD%" (
    echo Error: MSBuild.exe not found at %MSBUILD%
    exit /b 1
)

echo Building Irrlicht Engine (x64 Debug, CONFORM_TEST_DX9)...
echo.

copy Directory_DX9.Build.props Directory.Build.props
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