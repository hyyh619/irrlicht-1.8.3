import os
import sys
import time
import json

from hlsl_interpreter import HLSLInterpreter
from rasterizer import Rasterizer
from d3d import D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST


def main():
    if len(sys.argv) < 2:
        print("Usage: python render.py <config.json>")
        print("Config JSON should contain: hlsl_file_path, csv_folder_path, log_file_path")
        config_path = './wrong_constant_attenuation.json'
    else:
        config_path = sys.argv[1]

    if not os.path.exists(config_path):
        print(f"Error: Config file not found: {config_path}")
        sys.exit(1)

    config = {}
    with open(config_path, 'r', encoding='utf-8') as f:
        config = json.load(f)

    hlsl_file_path = config.get('hlsl_file_path', '')
    csv_folder_path = config.get('csv_folder_path', '')
    log_file_path = config.get('log_file_path', 'hlsl_interpreter.log')
    log_file_mode = config.get('log_file_mode', 'a')
    print_sequence = config.get('print_sequence', 1)
    log_to_file = config.get('log_to_file', True)
    printSyntaxTree = config.get('printSyntaxTree', True)
    print_interpreter_result = config.get('print_interpreter_result', True)
    float_tolerance = config.get('float_tolerance', 0.0001)
    output_struct_name = config.get('output_struct_name', 'VS_OUTPUT')
    execute_count = config.get('execute_count', None)
    max_workers = config.get('max_workers', 1)
    primitive_topology = config.get('primitive_topology', D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
    mesh_view_enabled = config.get('mesh_view_enabled', False)

    if not hlsl_file_path:
        print("Error: hlsl_file_path not specified in config")
        sys.exit(1)

    if not os.path.exists(hlsl_file_path):
        print(f"Error: HLSL file not found: {hlsl_file_path}")
        sys.exit(1)

    if csv_folder_path and not os.path.exists(csv_folder_path):
        print(f"Error: CSV folder not found: {csv_folder_path}")
        sys.exit(1)

    interpreter = HLSLInterpreter(
        log_to_file=log_to_file,
        log_file_path=log_file_path,
        log_file_mode=log_file_mode,
        print_sequence=print_sequence,
        printSyntaxTree=printSyntaxTree,
        print_interpreter_result=print_interpreter_result,
        max_workers=max_workers,
        primitive_topology=primitive_topology)

    if mesh_view_enabled:
        interpreter.enable_mesh_view(True)

    total_start = time.time()

    interpret_start = time.time()
    interpreter.interpret(hlsl_file_path, csv_folder_path)
    interpret_time = time.time() - interpret_start

    if mesh_view_enabled and interpreter._mesh_view:
        interpreter._mesh_view.set_hlsl_interpreter(interpreter, "main", "VS_INPUT")

    golden_csv_path = os.path.join(csv_folder_path, 'VS_OUTPUT.csv') if csv_folder_path else None
    load_golden_start = time.time()
    if golden_csv_path and os.path.exists(golden_csv_path):
        interpreter.load_vs_output_golden_from_csv(golden_csv_path)
    load_golden_time = time.time() - load_golden_start

    if mesh_view_enabled:
        interpreter.log_output("Displaying input mesh before executeVS...")
        interpreter.show_input_mesh("VS_INPUT")

    # 1. Execute VS
    execute_start = time.time()
    results = interpreter.executeVS("vs_main", "VS_INPUT", execute_count=execute_count)
    execute_time = time.time() - execute_start

    if mesh_view_enabled and results:
        interpreter.log_output("Displaying result mesh after executeVS...")
        interpreter.show_result_mesh(results)

    # Execute rasterization
    r = Rasterizer("rasterizer_param.json")
    pixels = r.rasterize(results, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
    interpreter._mesh_view.set_rasterizer_pixels(pixels)

    if mesh_view_enabled and pixels:
        interpreter.log_output("Displaying pixels after rasterizer...")
        interpreter._mesh_view._draw_rasterizer_pixels()

    # 3. 执行PS（需要提供纹理和采样器配置文件路径）
    interpreter.executePS("ps_main", "PS_INPUT", pixels, 
                    texture_config_path, sampler_config_path)

    # 在MeshView中显示
    if mesh_view_enabled and pixels:
        mesh_view.set_rasterizer_pixels(pixels)  # 更新后的pixels已包含ps_output_color

    if interpreter.print_interpreter_result:
        interpreter.log_output("HLSL Interpreter Result:")
        interpreter.log_output("=" * 40)
        if results:
            for idx, result in enumerate(results):
                interpreter.log_output(f"\n--- Row {idx} ---")
                if result:
                    for key, value in result.items():
                        if isinstance(value, list):
                            if len(value) == 4:
                                interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
                            elif len(value) == 3:
                                interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
                            elif len(value) == 2:
                                interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}]")
                            else:
                                interpreter.log_output(f"{key}: {value}")
                        else:
                            interpreter.log_output(f"{key}: {value}")
        else:
            interpreter.log_output("No result produced")

        if results and results[-1] and 'Color' in results[-1]:
            color = results[-1]['Color']
            if color and isinstance(color, list) and len(color) == 4:
                interpreter.log_output("\nFinal Output Color (RGBA):")
                interpreter.log_output(f"  R: {color[0]:.4f}")
                interpreter.log_output(f"  G: {color[1]:.4f}")
                interpreter.log_output(f"  B: {color[2]:.4f}")
                interpreter.log_output(f"  A: {color[3]:.4f}")
            else:
                interpreter.log_output(f"\nColor result: {color}")

        interpreter.log_output("\n" + "=" * 40)
    interpreter.log_output("Comparing with golden data...")
    interpreter.log_output("=" * 40)
    compare_start = time.time()
    interpreter.compare_vs_output_with_golden(results, output_struct_name=output_struct_name, float_tolerance=float_tolerance, execute_count=execute_count)
    compare_time = time.time() - compare_start

    total_time = time.time() - total_start

    interpreter.log_output("\n" + "=" * 40)
    interpreter.log_output("Timing Summary:")
    interpreter.log_output("=" * 40)
    interpreter.log_output(f"interpreter.interpret():             {interpret_time:.4f}s")
    interpreter.log_output(f"interpreter.load_vs_output_golden_from_csv(): {load_golden_time:.4f}s")
    interpreter.log_output(f"interpreter.executeVS():           {execute_time:.4f}s")
    interpreter.log_output(f"compare_vs_output_with_golden():    {compare_time:.4f}s")
    interpreter.log_output(f"Total execution time:               {total_time:.4f}s")

    while True:
        user_input = input("\nEnter 'x' to exit, 'o' to open MeshView, 'r' to rerun executeVS: ")
        user_input = user_input.strip().lower()
        if user_input == 'x':
            interpreter._mesh_view.close()
            break
        elif user_input == 'o':
            if interpreter._mesh_view:
                interpreter._mesh_view.show(blocking=False)
                interpreter.log_output("MeshView reopened")
        elif user_input == 'r':
            results = []
            execute_start = time.time()
            results = interpreter.executeVS("main", "VS_INPUT", execute_count=execute_count)
            execute_time = time.time() - execute_start
            interpreter.log_output(f"Re-executed executeVS in {execute_time:.4f}s")
            if mesh_view_enabled and results:
                interpreter.log_output("Displaying result mesh after re-execution...")
                interpreter.show_result_mesh(results)
            if interpreter.print_interpreter_result:
                interpreter.log_output("HLSL Interpreter Result (re-run):")
                interpreter.log_output("=" * 40)
                if results:
                    for idx, result in enumerate(results):
                        interpreter.log_output(f"\n--- Row {idx} ---")
                        if result:
                            for key, value in result.items():
                                if isinstance(value, list):
                                    if len(value) == 4:
                                        interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
                                    elif len(value) == 3:
                                        interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
                                    elif len(value) == 2:
                                        interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}]")
                                    else:
                                        interpreter.log_output(f"{key}: {value}")
                                else:
                                    interpreter.log_output(f"{key}: {value}")
                else:
                    interpreter.log_output("No result produced")
                interpreter.log_output("=" * 40)


if __name__ == '__main__':
    main()