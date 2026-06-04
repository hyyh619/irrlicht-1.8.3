import os
import sys
import time
import json
import zipfile
import tempfile
import shutil

from hlsl_interpreter import HLSLInterpreter
from rasterizer import Rasterizer
from d3d import D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
from output_merger import Depth
import d3d


def print_and_compare_results(interpreter, results, output_struct_name, float_tolerance, execute_count, interpret_time, load_golden_time, execute_time, total_start):
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


def _make_interpreter(config: dict, shader_stage: int = d3d.SHADER_STAGE_VS, log_file_path: str = None, primitive_topology: int = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
                    texture_exec=None, texture_desc_list=None, sampler_list=None) -> HLSLInterpreter:
    """Create an HLSLInterpreter from config dict."""
    config_log_file_mode = config.get('log_file_mode', 'a')
    if shader_stage != d3d.SHADER_STAGE_VS and shader_stage != d3d.SHADER_STAGE_CS:
        config_log_file_mode = 'a'

    return HLSLInterpreter(
        log_to_file=config.get('log_to_file', True),
        log_file_path=log_file_path or config.get('log_file_path', 'hlsl_interpreter.log'),
        log_file_mode=config_log_file_mode,
        print_sequence=config.get('print_sequence', 1),
        printSyntaxTree=config.get('printSyntaxTree', True),
        print_interpreter_result=config.get('print_interpreter_result', True),
        max_workers=config.get('max_workers', 1),
        primitive_topology=primitive_topology,
        texture_list=[texture_exec] if texture_exec else [],
        texture_desc_list=texture_desc_list or [],
        sampler_list=sampler_list or [],
    )


def _run_zip_workflow(config: dict, data_path: str, config_path: str):
    """Execute the full rendering pipeline from a zip archive."""
    config_dir = os.path.dirname(os.path.abspath(config_path))
    if os.path.isabs(data_path):
        abs_data_path = data_path
    elif os.path.exists(data_path):
        abs_data_path = os.path.abspath(data_path)
    else:
        abs_data_path = os.path.join(config_dir, data_path)

    if not os.path.exists(abs_data_path):
        print(f"Error: data_path not found: {abs_data_path}")
        sys.exit(1)

    temp_dir = tempfile.mkdtemp(prefix='hlsl_interp_')
    try:
        print(f"Extracting {abs_data_path} ...")
        with zipfile.ZipFile(abs_data_path, 'r') as z:
            z.extractall(temp_dir)

        # Find top-level folder inside the zip
        items = os.listdir(temp_dir)
        if len(items) == 1 and os.path.isdir(os.path.join(temp_dir, items[0])):
            data_folder = os.path.join(temp_dir, items[0])
        else:
            data_folder = temp_dir

        _execute_pipeline(config, config_path, data_folder)
    finally:
        shutil.rmtree(temp_dir, ignore_errors=True)


def _execute_pipeline(config: dict, config_path: str, data_folder: str):
    """Execute VS → Rasterizer → Depth → PS pipeline from extracted data folder."""
    # Config parameters
    log_file_path = config.get('log_file_path', '')
    float_tolerance = config.get('float_tolerance', 0.0001)
    execute_count = config.get('execute_count', None)
    if execute_count == -1:
        execute_count = None
    early_z = config.get('early_z', True)
    mesh_view_enabled = config.get('mesh_view_enabled', False)
    primitive_topology = config.get('primitive_topology', D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)

    # Resolve log path relative to config
    if log_file_path:
        config_dir = os.path.dirname(os.path.abspath(config_path))
        log_file_path = log_file_path if os.path.isabs(log_file_path) else os.path.join(config_dir, log_file_path)

    # Data file paths
    vs_hlsl = os.path.join(data_folder, 'VS_shader.hlsl')
    ps_hlsl = os.path.join(data_folder, 'PS_shader.hlsl')
    vs_cb_csv = os.path.join(data_folder, 'VS_constant_buffers.csv')
    ps_cb_csv = os.path.join(data_folder, 'PS_constant_buffers.csv')
    vs_sig_csv = os.path.join(data_folder, 'VS_input_output_signature.csv')
    ps_sig_csv = os.path.join(data_folder, 'PS_input_output_signature.csv')
    ia_vertex_csv = os.path.join(data_folder, 'ia_vertex_data.csv')
    pipeline_state_csv = os.path.join(data_folder, 'pipeline_state.csv')
    golden_vs_csv = os.path.join(data_folder, 'MeshOut_vs_mesh.csv')

    total_start = time.time()

    # ============================================================
    # VS setup and execution
    # ============================================================
    vs_interp = _make_interpreter(config, d3d.SHADER_STAGE_VS, log_file_path, primitive_topology)

    if not os.path.exists(vs_hlsl):
        print(f"Error: VS shader not found: {vs_hlsl}")
        return

    with open(vs_hlsl, 'r', encoding='utf-8') as f:
        vs_code_raw = f.read()
    vs_code = vs_interp.preprocess_hlsl(vs_code_raw)
    vs_interp.hlsl_code = vs_code

    # Parse cbuffers, texture/sampler bindings, and functions from VS code
    vs_interp._parse_texture_and_sampler_bindings(vs_code)
    for cb_match in vs_interp.patterns['cbuffer_finditer'].finditer(vs_code):
        cb_def = vs_interp.parse_cbuffer(cb_match.group())
        if cb_def:
            vs_interp.cbuffers[cb_def.name] = cb_def
    vs_interp.parse_all_functions(vs_code)

    # Load cbuffer data
    if os.path.exists(vs_cb_csv):
        vs_interp.load_all_cbuffers_from_combined_csv(vs_cb_csv)

    # Load VS signature
    vs_sig = vs_interp.load_signature_from_csv(vs_sig_csv)

    # Parse VS main function parameters
    vs_params = vs_interp.parse_main_params_with_semantics(vs_code, 'main')
    if not vs_params:
        print("Error: Could not parse VS main() parameters")
        return

    vs_input_params = vs_params['inputs']
    vs_output_params = vs_params['outputs']

    vs_interp.map_params_to_signature(vs_input_params, vs_sig['inputs'])
    vs_interp.map_params_to_signature(vs_output_params, vs_sig['outputs'])

    vs_interp.log_output(f"VS inputs:  {[(p['name'], p['type'], p['semantic']) for p in vs_input_params]}")
    vs_interp.log_output(f"VS outputs: {[(p['name'], p['type'], p['semantic'], 'slot', p['slot']) for p in vs_output_params]}")

    # Load vertex data
    vertex_data = vs_interp.load_ia_vertex_data(ia_vertex_csv, vs_input_params)
    vs_interp.log_output(f"Loaded {len(vertex_data)} vertices from ia_vertex_data.csv")

    # Load golden VS output (optional)
    golden_vs_rows = []
    if os.path.exists(golden_vs_csv):
        golden_vs_rows = vs_interp.load_vs_golden_from_mesh_csv(golden_vs_csv, vs_output_params)
        vs_interp.log_output(f"Loaded {len(golden_vs_rows)} golden VS output rows")

    # Execute VS
    vs_interp.log_output("=" * 50)
    vs_interp.log_output("Executing Vertex Shader...")
    vs_start = time.time()
    vs_results = vs_interp.executeVS_with_params(
        'main', vs_input_params, vs_output_params, vertex_data, execute_count=execute_count
    )
    vs_time = time.time() - vs_start
    vs_interp.log_output(f"VS executed {len(vs_results)} vertices in {vs_time:.4f}s")

    # Print sample results
    if vs_interp.print_interpreter_result and vs_results:
        vs_interp.log_output("\nSample VS output (first row):")
        first = vs_results[0]
        for k, v in first.items():
            if isinstance(v, list):
                vs_interp.log_output(f"  {k}: [{', '.join(f'{x:.4f}' for x in v)}]")
            elif isinstance(v, (int, float)):
                vs_interp.log_output(f"  {k}: {v:.4f}")

    # Compare with golden
    if golden_vs_rows:
        vs_interp.log_output("\nComparing VS output with golden data...")
        vs_interp.compare_vs_output_with_golden_params(
            vs_results, vs_output_params, golden_vs_rows,
            float_tolerance=float_tolerance,
            execute_count=execute_count,
        )

    # ============================================================
    # Rasterizer
    # ============================================================
    rast = Rasterizer()
    if os.path.exists(pipeline_state_csv):
        topo_from_csv = rast.load_config_from_pipeline_state_csv(pipeline_state_csv)
        # Use config override if explicitly set, otherwise use CSV value
        if 'primitive_topology' not in config and topo_from_csv is not None:
            primitive_topology = topo_from_csv

    vs_interp.log_output(f"\nRasterizing {len(vs_results)} vertices (topology={primitive_topology})...")
    rast_start = time.time()
    pixels = rast.rasterize(vs_results, primitive_topology)
    rast_time = time.time() - rast_start
    vs_interp.log_output(f"Rasterized → {len(pixels)} pixels in {rast_time:.4f}s")

    # Depth/stencil
    depth = Depth()
    if early_z:
        pixels = depth.execute(pixels, early_z=True)
        vs_interp.log_output(f"Early-Z: {len(pixels)} pixels after depth test")

    pixels_for_ps = pixels

    # ============================================================
    # PS setup and execution
    # ============================================================
    if os.path.exists(ps_hlsl) and pixels_for_ps:
        ps_interp = _make_interpreter(config, d3d.SHADER_STAGE_PS, log_file_path)

        with open(ps_hlsl, 'r', encoding='utf-8') as f:
            ps_code_raw = f.read()
        ps_code = ps_interp.preprocess_hlsl(ps_code_raw)
        ps_interp.hlsl_code = ps_code

        ps_interp._parse_texture_and_sampler_bindings(ps_code)
        for cb_match in ps_interp.patterns['cbuffer_finditer'].finditer(ps_code):
            cb_def = ps_interp.parse_cbuffer(cb_match.group())
            if cb_def:
                ps_interp.cbuffers[cb_def.name] = cb_def
        ps_interp.parse_all_functions(ps_code)

        if os.path.exists(ps_cb_csv):
            ps_interp.load_all_cbuffers_from_combined_csv(ps_cb_csv)

        ps_params = ps_interp.parse_main_params_with_semantics(ps_code, 'main')
        if ps_params:
            ps_input_params = ps_params['inputs']
            ps_output_params = ps_params['outputs']

            if os.path.exists(ps_sig_csv):
                ps_sig = ps_interp.load_signature_from_csv(ps_sig_csv)
                ps_interp.map_params_to_signature(ps_input_params, ps_sig['inputs'])
                ps_interp.map_params_to_signature(ps_output_params, ps_sig['outputs'])

            ps_interp.log_output(f"\nExecuting Pixel Shader on {len(pixels_for_ps)} pixels...")
            ps_start = time.time()
            ps_interp.executePS_with_params(
                'main', ps_input_params, ps_output_params, pixels_for_ps, ps_code=ps_code
            )
            ps_time = time.time() - ps_start
            ps_interp.log_output(f"PS executed in {ps_time:.4f}s")

    if not early_z:
        pixels = depth.execute(pixels, early_z=False)
        vs_interp.log_output(f"Late-Z: {len(pixels)} pixels after depth test")

    total_time = time.time() - total_start
    vs_interp.log_output("\n" + "=" * 50)
    vs_interp.log_output("Pipeline Summary:")
    vs_interp.log_output(f"  VS:          {len(vs_results)} vertices in {vs_time:.4f}s")
    vs_interp.log_output(f"  Rasterizer:  {len(rast.get_pixels())} pixels in {rast_time:.4f}s")
    vs_interp.log_output(f"  Total:       {total_time:.4f}s")


def _run_legacy_workflow(config: dict, config_path: str):
    """Original struct-based workflow for old JSON format."""
    config_dir = os.path.dirname(os.path.abspath(config_path))

    def resolve(p):
        return p if (not p or os.path.isabs(p)) else os.path.join(config_dir, p)

    hlsl_file_path = resolve(config.get('hlsl_file_path', ''))
    csv_folder_path = resolve(config.get('csv_folder_path', ''))
    log_file_path = resolve(config.get('log_file_path', 'hlsl_interpreter.log'))
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
    texture_desc_path = resolve(config.get('texture_desc', ''))
    sampler_config_path = resolve(config.get('sampler_config', ''))
    depth_stencil_config_path = resolve(config.get('depth_stencil_config', ''))
    early_z = config.get("early_z", True)

    texture_desc_list = []
    sampler_list = []
    texture_exec = None
    if texture_desc_path and sampler_config_path:
        from texture import TextureDesc, Sampler, Texture
        with open(texture_desc_path, 'r', encoding='utf-8') as f:
            texture_data = json.load(f)
        with open(sampler_config_path, 'r', encoding='utf-8') as f:
            sampler_data = json.load(f)
        for tex_id in texture_data:
            texture_desc_list.append(TextureDesc.from_config(texture_desc_path, int(tex_id)))
        for samp_id in sampler_data:
            sampler_list.append(Sampler.from_config(sampler_config_path, int(samp_id)))
        texture_exec = Texture()

    interpreter = HLSLInterpreter(
        log_to_file=log_to_file,
        log_file_path=log_file_path,
        log_file_mode=log_file_mode,
        print_sequence=print_sequence,
        printSyntaxTree=printSyntaxTree,
        print_interpreter_result=print_interpreter_result,
        max_workers=max_workers,
        primitive_topology=primitive_topology,
        texture_list=[texture_exec] if texture_exec else [],
        texture_desc_list=texture_desc_list,
        sampler_list=sampler_list)

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

    execute_start = time.time()
    results = interpreter.executeVS("vs_main", "VS_INPUT", execute_count=execute_count)
    execute_time = time.time() - execute_start

    if mesh_view_enabled:
        interpreter.log_output("Displaying input mesh before executeVS...")
        interpreter.show_input_mesh("VS_INPUT")

    print_and_compare_results(interpreter, results, output_struct_name, float_tolerance, execute_count, interpret_time, load_golden_time, execute_time, total_start)

    if mesh_view_enabled and results:
        interpreter.log_output("Displaying result mesh after executeVS...")
        interpreter.show_result_mesh(results)

    # Rasterization
    raster_param_path = resolve(config.get('rasterizer_param', 'rasterizer_param.json'))
    r = Rasterizer(raster_param_path if os.path.exists(raster_param_path) else None)
    pixels = r.rasterize(results, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)

    depth = Depth(depth_stencil_config_path) if depth_stencil_config_path and os.path.exists(depth_stencil_config_path) else Depth()

    if early_z:
        depth_pixels = depth.execute(pixels, early_z=True)
        interpreter.log_output(f"Early-Z: Depth processed {len(pixels)} rasterizer pixels to {len(depth_pixels)} pixels")
    else:
        interpreter.log_output(f"Late-Z: Depth will process pixels after PS")
        depth_pixels = pixels

    if mesh_view_enabled and interpreter._mesh_view:
        interpreter._mesh_view.set_rasterizer_pixels(depth_pixels)

    pixels_for_ps = depth_pixels if early_z else pixels

    interpreter.executePS("ps_main", "PS_INPUT_BASIC", pixels_for_ps)

    if not early_z:
        depth_pixels = depth.execute(pixels, early_z=False)
        interpreter.log_output(f"Late-Z: Depth processed {len(pixels)} PS pixels to {len(depth_pixels)} pixels")

    if mesh_view_enabled and interpreter._mesh_view:
        interpreter._mesh_view.set_rasterizer_pixels(depth_pixels)
        interpreter._mesh_view.set_output_merger_pixels(depth_pixels)
        interpreter._mesh_view._draw_rasterizer_pixels()
        interpreter._mesh_view._draw_pixel_shader_pixels()
        interpreter._mesh_view._draw_output_merger_pixels()

    while True:
        user_input = input("\nEnter 'x' to exit, 'o' to open MeshView, 'r' to rerun executeVS: ")
        user_input = user_input.strip().lower()
        if user_input == 'x':
            if interpreter._mesh_view:
                interpreter._mesh_view.close()
            break
        elif user_input == 'o':
            if interpreter._mesh_view:
                interpreter._mesh_view.show(blocking=False)
        elif user_input == 'r':
            execute_start = time.time()
            results = interpreter.executeVS("main", "VS_INPUT", execute_count=execute_count)
            execute_time = time.time() - execute_start
            interpreter.log_output(f"Re-executed executeVS in {execute_time:.4f}s")


def main():
    if len(sys.argv) < 2:
        print("Usage: python render.py <config.json>")
        config_path = './wrong_constant_attenuation.json'
    else:
        config_path = sys.argv[1]

    if not os.path.exists(config_path):
        print(f"Error: Config file not found: {config_path}")
        sys.exit(1)

    with open(config_path, 'r', encoding='utf-8') as f:
        config = json.load(f)

    data_path = config.get('data_path', '')
    if data_path:
        _run_zip_workflow(config, data_path, config_path)
    else:
        _run_legacy_workflow(config, config_path)


if __name__ == '__main__':
    main()
