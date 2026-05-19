# HLSL Interpreter Development Document

## Overview

This document describes the development history and technical details of the HLSL Interpreter - a Python-based tool that parses and executes HLSL (High-Level Shading Language) vertex shader code for debugging and validation purposes in the Irrlicht Engine D3D11 driver development.

**Development Timeline**: 49 implementation sessions (April-May 2026)
**Total Commits**: 87 (skipping 46 pure documentation commits)
**Main Files**: `hlsl_interpreter.py` (~2452 lines), `mesh_view.py` (~883 lines)

---

## Architecture Overview

### Directory Structure

```
hlsl_interpreter/
├── hlsl_interpreter.py      # Main interpreter
├── mesh_view.py             # 3D visualization GUI
├── animation_config.json    # Animation timing config
├── test_data.json           # Legacy test data
├── tests/                   # Debug test scripts
│   ├── debug_test.py
│   ├── debug_test2.py
│   └── ...
├── specular_too_shining/     # Test case data
├── constant_buffer_attenuation_wrong/
└── Sessions/hlsl-interpreter/  # Development session notes

Prompt/
├── hlsl-interpreter-prompt.md  # Design decisions
└── Sessions/hlsl-interpreter/  # 49 session notes (hlsl-step1 to hlsl-step49)
```

### Core Components

#### 1. SyntaxTreeNode (lines 35-98)

Base class for all syntax tree nodes with the following node types:

```python
class SyntaxTreeNode:
    def __init__(self, node_type: str, value=None):
        self.node_type = node_type  # 'value', 'function', 'binary_op', etc.
        self.value = value          # Actual data
        self.children = []          # Child nodes
```

| Node Type | Description |
|-----------|-------------|
| `value` | Variable/constant values |
| `function` | Function calls (transpose, normalize, mul, etc.) |
| `binary_op` | Binary operations (+, -, *, /, ., etc.) |
| `unary_op` | Unary operations (-, !) |
| `cast` | Type casting ((float3x3)expr) |
| `ternary` | Conditional (a ? b : c) |

#### 2. SyntaxTreeParser (lines 101-398)

Parses HLSL expressions into syntax trees with proper operator precedence.

**Operator Precedence (low to high)**:
```python
{
    '||': 1, '&&': 2,
    '==': 3, '!=': 3,
    '<': 4, '>': 4, '<=': 4, '>=': 4,
    '+': 5, '-': 5,
    '*': 6, '/': 6,
    '.': 7   # Highest - swizzle access
}
```

**Parsing Stages**:
1. Type casts: `(float3x3)expr`
2. Parenthesized expressions: `(expr)`
3. Ternary operator: `a ? b : c`
4. Binary operators with right-to-left associativity
5. Function calls: `float2/3/4(...)`, `mul(...)`, `transpose(...)`
6. Variable/constant values

#### 3. Data Classes

```python
@dataclass
class FieldDefinition:
    field_type: str      # float3, float4x4, etc.
    name: str           # Variable name
    semantic: str       # POSITION, NORMAL, COLOR, etc.
    data: List[Any] = None  # Actual data

@dataclass
class StructDefinition:
    name: str
    fields: List[FieldDefinition]

@dataclass
class CbufferDefinition:
    name: str
    fields: List[FieldDefinition]
```

#### 4. HLSLInterpreter (lines 430-2452)

Main interpreter class with key methods:

| Method | Description |
|--------|-------------|
| `interpret(hlsl_file_path, csv_folder_path)` | Parse HLSL, load CSV data |
| `executeVS(main_func, vs_input, code, execute_count)` | Execute vertex shader |
| `execute_main_function(...)` | Execute function with row data |
| `evaluate_expression(expr, local_vars)` | Evaluate expression via syntax tree |
| `evaluate_syntax_tree(node, local_vars)` | Execute syntax tree node |
| `execute_function_node(node, local_vars)` | Handle function calls |
| `get_value(name, local_vars)` | Retrieve variable with swizzle support |
| `compare_vs_output_with_golden(...)` | Validate against golden data |

---

## Data Types

```python
DATA_TYPE_LIST = [
    # Matrices
    'float4x4', 'float3x3',
    # Float vectors
    'float4', 'float3', 'float2', 'float',
    # Unsigned integers
    'uint4', 'uint3', 'uint2', 'uint',
    # Signed integers
    'int4', 'int3', 'int2', 'int',
    # Boolean
    'bool'
]
```

### Type Conversion Rules

| Source | Target | Conversion |
|--------|--------|------------|
| float | float2 | Replicate to vector |
| float | float3 | Replicate to vector |
| float | float4 | Replicate to vector |
| float3x3 | float4x4 | Pad to 4x4 (bottom-right = 1) |

---

## Supported Built-in Functions

| Function | Syntax | Description |
|----------|--------|-------------|
| Vector constructors | `float2(x, y)`, `float3(x, y, z)`, `float4(x, y, z, w)` | Create vectors |
| Matrix transpose | `transpose(m)` | Swap rows and columns |
| Vector normalization | `normalize(v)` | Scale to unit length |
| Vector magnitude | `length(v)` | Euclidean length |
| Dot product | `dot(a, b)` | Sum of element products |
| Cross product | `cross(a, b)` | Vector cross product |
| Reflection | `reflect(I, N)` | Reflect vector around normal |
| Multiplication | `mul(a, b)` | Matrix-vector or vector-matrix |
| Power | `pow(base, exp)` | Exponentiation |
| Min/Max | `max(a, b)`, `min(a, b)` | Clamp values |
| Absolute | `abs(v)` | Absolute value |
| Trigonometry | `sin(v)`, `cos(v)` | Sine/cosine |

---

## Supported Operators

### Arithmetic Operators
| Operator | Description |
|----------|-------------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division |
| `-` (unary) | Negation |

### Comparison Operators
| Operator | Description |
|----------|-------------|
| `==` | Equal |
| `!=` | Not equal |
| `<` | Less than |
| `>` | Greater than |
| `<=` | Less or equal |
| `>=` | Greater or equal |

### Logical Operators
| Operator | Description |
|----------|-------------|
| `&&` | Logical AND |
| `||` | Logical OR |
| `!` | Logical NOT |

### Ternary Operator
```hlsl
condition ? value_if_true : value_if_false
```

### Swizzle Operator
```hlsl
vector.xxx        // Replicate x
vector.xy         // Select x, y
vector.xyz        // Select x, y, z
vector.rgba       // Same as xyzw
vector.xyzw       // Select all four
```

---

## Development Timeline

### Phase 1: Foundation (Steps 1-9)

| Step | Feature | Description |
|------|---------|-------------|
| 1 | Initial code | Basic HLSL execution framework with struct/cbuffer parsing |
| 2-3 | Struct/cbuffer refinements | Improved parsing of HLSL structs |
| 4 | CSV data loading | Load struct/cbuffer data from CSV files |
| 5-7 | executeVS framework | Basic vertex shader execution |
| 8 | JSON configuration | Load settings from JSON config file |
| 9 | Syntax tree creation | Initial expression parsing into tree |

### Phase 2: Expression Evaluation (Steps 10-19)

| Step | Feature | Description |
|------|---------|-------------|
| 10-14 | execute_function refinement | Improved function execution and evaluation printing |
| 15 | **Syntax tree module** | Proper operator precedence implementation |
| 16-17 | Execution printing | Syntax tree operation debugging output |
| 18-19 | Matrix-vector multiplication fix | Column-major matrix handling, float3x3 cast support |

### Phase 3: Operators & Functions (Steps 20-29)

| Step | Feature | Description |
|------|---------|-------------|
| 20 | **Dot operator** | Vector element access (e.g., `Attenuation.x`) |
| 21 | Swizzle support | xyzw and rgba notation for vector components |
| 22-24 | Comments and casts | Float4x4/float3x3 cast, golden comparison output |
| 25-26 | Argument validation | Fixed max/min/dot argument count validation |
| 27-29 | All expressions via syntax tree | Fixed operator precedence |

### Phase 4: Control Flow & Performance (Steps 30-39)

| Step | Feature | Description |
|------|---------|-------------|
| 30-32 | Print controller | Sequence-based logging control |
| 33-34 | JSON config | Timing instrumentation via config |
| 35 | Function body parsing | Fixed parsing of multi-statement function bodies |
| 36-38 | Two-char operator fix | Proper handling of `<=`, `>=` operators |
| 39 | **Function cache** | Performance optimization to avoid re-parsing |

### Phase 5: Advanced Features (Steps 40-49)

| Step | Feature | Description |
|------|---------|-------------|
| 40-41 | VS execution count | Per-vertex execution tracking |
| 41-43 | **If-cond-else support** | Conditional execution in shaders |
| 43-44 | Output swizzle fix | Fixed `output.Color.xxx` syntax |
| 44-45 | **Thread workers** | Multi-threaded execution |
| 46-48 | **MeshView GUI** | Dual-window visualization |
| 49 | Animation rendering | Vertex-by-vertex animation playback |

---

## Key Technical Decisions

### 1. Row-Vector Math

HLSL uses row-vector notation, so the interpreter multiplies matrix by vector:
```python
def mul_matrix_vector(self, m: List[List[float]], v: List[float]]) -> List[float]:
    result = [0.0] * num_cols
    for j in range(num_cols):
        result[j] = sum(v[i] * m[i][j] for i in range(len(v)))
    return result
```

This means `mul(float4(pos, 1.0), transpose(WorldViewProj))` works correctly.

### 2. Function Cache

```python
self._parsed_func_cache = {}  # Cache key: f"{output_struct}_{main_func}_{input_struct}"
```

Avoids re-parsing the same function body, significantly improving performance for repeated executions.

### 3. Print Sequence Control

```python
self._eval_counter += 1
self._should_print = ((self._eval_counter - 1) % self.print_sequence == 0)
```

Reduces log output by only printing every N executions (controlled via JSON config).

### 4. Swizzle Implementation

The `.` operator has highest precedence and is handled via `apply_swizzle()`:
- Single component: `.x` returns scalar
- Multiple components: `.xyz` returns list of 3 elements
- Supports both xyzw and rgba notation

### 5. Multi-threaded Execution

```python
with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
    futures = [executor.submit(execute_row, i) for i in range(execute_count)]
```

ThreadPoolExecutor for parallel vertex processing. Configurable worker count via JSON.

### 6. Tolerance-based Comparison

```python
def compare_with_tolerance(self, expected, actual, tolerance=1e-6) -> bool:
    if isinstance(expected, (int, float)):
        return abs(expected - actual) <= tolerance
    # Recursive for vectors/matrices
```

Golden output comparison with configurable tolerance to handle floating-point precision issues.

---

## I/O and Validation

### Input Format (CSV)

CSV files with component columns (x, y, z, w) for vector data:

```csv
WorldViewProj_0,WorldViewProj_1,WorldViewProj_2,WorldViewProj_3
1.0,0.0,0.0,0.0
0.0,1.0,0.0,0.0
0.0,0.0,1.0,0.0
0.0,0.0,0.0,1.0
```

### Output Format (CSV)

Same format as input, generated by vertex shader execution.

### JSON Configuration

```json
{
    "print_sequence": 100,
    "max_workers": 4,
    "tolerance": 1e-6,
    "log_file": true,
    "log_level": "INFO"
}
```

### Golden Comparison

1. Execute shader on test input
2. Compare output against known-good `.csv` file
3. Report PASS/FAIL with tolerance-based matching

---

## MeshView GUI Features

| Feature | Description |
|---------|-------------|
| Dual windows | Left: input mesh, Right: output mesh |
| Rotation controls | Mouse drag to rotate |
| Zoom controls | Mouse wheel |
| Pan controls | Middle mouse button |
| Normal vector display | Toggle to show vertex normals |
| Animation playback | Step-by-step vertex rendering |
| Frame stepping | Manual frame advance |

### Supported Primitive Topologies

| Topology | Description |
|----------|-------------|
| POINTLIST | Points only |
| LINELIST | Line pairs |
| LINESTRIP | Connected lines |
| TRIANGLELIST | Individual triangles |
| TRIANGLESTRIP | Connected triangles |
| TRIANGLEFAN | Triangle fan (deprecated in D3D11) |

---

## Test Cases

The interpreter includes multiple test cases for validation:

| Test Case | Description |
|-----------|-------------|
| `color-correct-ninjia-of-collision` | Correct color output with collision detection |
| `color-correct-ninjia-of-collision-if-cond` | With conditional statements |
| `color-correct-ninjia-of-collision-if-cond-multi-stmts` | Multiple statements in if blocks |
| `specular_too_shining` | Specular lighting bug case |
| `wrong_constant_attenuation` | Light attenuation bug case |
| `constant_buffer_attenuation_wrong` | Constant buffer data issue |

---

## File Reference

| File | Lines | Description |
|------|-------|-------------|
| `hlsl_interpreter/hlsl_interpreter.py` | ~2452 | Main interpreter |
| `hlsl_interpreter/mesh_view.py` | ~883 | 3D visualization |
| `hlsl_interpreter/animation_config.json` | - | Animation timing |

### Session Notes

Development sessions are documented in `Sessions/hlsl-interpreter/`:
- `hlsl-step1-init-code.md` through `hlsl-step49-fix-animation-failure.md`

Session naming convention: `hlsl-step[N]-[description].md`

---

## Usage Examples

### Basic Interpretation

```python
interpreter = HLSLInterpreter()
interpreter.interpret(
    hlsl_file_path="vertex_shader.hlsl",
    csv_folder_path="test_data/"
)
```

### Execute with Count

```python
interpreter.executeVS(
    main_func="main",
    vs_input=vs_input_data,
    code=hlsl_code,
    execute_count=1000
)
```

### Compare with Golden

```python
result = interpreter.compare_vs_output_with_golden(
    golden_csv="VS_OUTPUT.csv",
    tolerance=1e-6
)
# Returns: (passed: bool, diff_count: int, details: list)
```