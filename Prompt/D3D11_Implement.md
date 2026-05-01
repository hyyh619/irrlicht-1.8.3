# 1

对照source\Irrlicht\CD3D9*.cpp, source\Irrlicht\CD3D9*.h的代码，补全CD3D11*.cpp和CD3D11*.h的实现

# 2
1. 仿照examples里面的CONFORM_TEST代码，增加一个CONFORM_TEST_DX11的代码，使用driverType是EDT_DIRECT3D11
2. 并重写build_debug_x64_conform.bat脚本，使其可以同时支持CONFORM_TEST和CONFORM_TEST_DX11
3. 重命名CONFORM_TEST为CONFORM_TEST_DX9