创建一个skill能够自动编译整个项目，运行测试列表，确认测试列表内的测试是否通过。
测试的列表如下
1. 01.HelloWorld
2. 02.Quake3Map

该skill需要执行下面的步骤进
1. 使用examples/build_debug_x64_conform.bat编译整个项目
2. 运行测试列表，获得screenshot.bmp文件
3. 使用Scripts/compare_screenshots.py逐个比较测试列表example生成的screenshot.bmp和其对应的golden图片。
    golden图片放置的位置以01.HelloWorld为例，放在如下位置examples/01.HelloWorld/Frame1-screenshot-golden.bmp。
4. 获得步骤3比较的结果，告诉我所有测试的结果。