# 1
## Prompts
1. render.py执行时输入的json文件格式变化
   a. 去掉了下列项目
        "hlsl_file_path": "./color-correct-ninjia-of-collision/VERTEX_SHADER_STANDARD_POINT.hlsl",
        "csv_folder_path": "./color-correct-ninjia-of-collision/",
        "rasterizer_param": "./color-correct-ninjia-of-collision/rasterizer_param.json",
        "sampler_config": "./color-correct-ninjia-of-collision/sampler_config.json",
        "texture_desc": "./color-correct-ninjia-of-collision/texture_desc.json",
        "depth_stencil_config": "./color-correct-ninjia-of-collision/depth_stencil_descriptor.json",
    b. 增加了
        "data_path": "./Cases/Collision-fix-constant-buffer-and-RdotV-zero_event475.zip",
    c. data_path给的是一个zip的压缩包，该压缩包包含：input vertex data, VS output data, texture data, hlsl source，每个shader stage阶段使用到的constant buffer data。这些data都以csv格式给出（除了texture data使用bitmap文件格式）。
    d. render.py需要先解压缩该压缩包，获取对应的数据。之前这些数据都是由输入的json文件通过每个独立的config项来指定，例如hlsl就是由hlsl_file_path来指定所有的shader stage阶段的代码。现在分成每个shader stage阶段有独立的hlsl文件。
2. VS 的input和output不是由struct定义，而是在main函数的参数中定义，如下
   void main( 
  float3 v0 : POSITION0,
  float3 v1 : NORMAL0,
  float4 v2 : COLOR0,
  float2 v3 : TEXCOORD0,
  out float4 o0 : SV_POSITION0,
  out float4 o1 : COLOR0,
  out float2 o2 : TEXCOORD0,
  out float2 p2 : TEXCOORD1,
  out float3 o3 : NORMAL0,
  out float3 o4 : WORLDPOS0)
  请实现对上述输入参数的解析
3. 通过 VS input signature，VS output signature(都需要从压缩包里的VS_input_output_signature.csv和PS_input_output_signature.csv来获取)来完成VS的input和output的数据流映射
4. PS的input和output也不是由struct定义，而是如下，请跟VS一样，完成输入数据和输出数据流的映射
void main( 
  float4 v0 : SV_POSITION0,
  float4 v1 : COLOR0,
  float2 v2 : TEXCOORD0,
  float2 w2 : TEXCOORD1,
  float3 v3 : NORMAL0,
  float3 v4 : WORLDPOS0,
  out float4 o0 : SV_TARGET0)
5. rasterizer state/blend state/depth-stencil state的对应配置设置文件改成pipeline_state.csv。请改成从这个文件解析
6. VS的输入数据的input layout定义文件是ia_input_layouts.csv定义，数据文件是ia_vertex_data.csv，请根据vs input signature和ia_input_layouts.csv来完成输入顶点数据到vs hlsl的输入数据的映射。

## Git commit: hlsl-interpreter: using DxRenderDoc dump zip file as input by Claude Code.


# 2 修复前序提交引入的bug
## Prompts
请查看render.py运行时的打印log文件output.log，有如下的Error打印
Error: Row 0 Color[0]: output=1.640544 golden=0.431490 diff=1.209054
Error: Row 0 Color[1]: output=1.640544 golden=0.431490 diff=1.209054
Error: Row 0 Color[2]: output=1.640544 golden=0.431490 diff=1.209054
Error: Row 0 WorldPos[0]: output=-61.638200 golden=11.282900 diff=72.921100
Error: Row 0 WorldPos[1]: output=11.282900 golden=-88.120250 diff=99.403150
Error: Row 0 WorldPos[2]: output=-88.120300 golden=-58.054070 diff=30.066230
VS执行完毕的结果与golden data的数据对比，Color/WorldPos错误。
1. 请修复该问题。
2. 修复代码后执行下列命令来验证修复是否成功。运行render.py后会生成output.log(该文件在Cases文件夹中)，请读取output.log内容来验证还有Error打印吗？如果还有Error，请继续修复，知道VS输出正确
   python.exe render.py ./Cases/Default.json

## Git commit: 


# 3
## Prompts
## Git commit: 


# 4
## Prompts
## Git commit: 


# 5
## Prompts
## Git commit: 


# 6
## Prompts
## Git commit: 


# 7
## Prompts
## Git commit: 


# 8
## Prompts
## Git commit: 


# 9
## Prompts
## Git commit: 
