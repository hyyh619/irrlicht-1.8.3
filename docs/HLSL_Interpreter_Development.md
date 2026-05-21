# HLSL Interpreter Development Document

## Overview

This document describes the development history and technical details of the HLSL Interpreter - a Python-based tool that parses and executes HLSL (High-Level Shading Language) vertex shader code for debugging and validation purposes in the Irrlicht Engine D3D11 driver development.

**Development Timeline**: April-May 2026
**Total Commits (all projects)**: 301 (from eaf611da to f0f5f72)
**hlsl-inter Commits**: 120 (from commits with "hlsl-inter:" prefix)
**hlsl-inter Effective Commits**: 107 (excluding commits with only .md files)
**Main Files**: `hlsl_interpreter.py` (~2313 lines), `hlsl_syntax_tree.py` (~288 lines), `mesh_view.py` (~1549 lines)

---

## Architecture Overview

### Core Files

| File | Lines | Description |
|------|-------|-------------|
| `hlsl_interpreter/hlsl_interpreter.py` | ~2313 | Main interpreter - parsing, execution, I/O |
| `hlsl_interpreter/hlsl_syntax_tree.py` | ~288 | Syntax tree node classes and parser |
| `hlsl_interpreter/mesh_view.py` | ~1549 | 3D mesh visualization GUI (Tkinter) |
| `hlsl_interpreter/animation_config.json` | - | Animation timing configuration |

### Directory Structure

```
hlsl_interpreter/
├── hlsl_interpreter.py           # Main interpreter
├── hlsl_syntax_tree.py           # Syntax tree parser
├── mesh_view.py                  # 3D visualization GUI
├── animation_config.json         # Animation timing
├── color-correct-ninjia-of-collision/    # Test case data
├── color-correct-ninjia-of-collision-if-cond/
├── color-correct-ninjia-of-collision-if-cond-multi-stmts/
├── specular_too_shining/         # Bug case: specular too shining
├── wrong_constant_attenuation/   # Bug case: wrong constant
├── constant_buffer_attenuation_wrong/
└── tests/                        # Debug test scripts
```

---

## Core Components

### 1. SyntaxTreeNode (hlsl_syntax_tree.py:112-172)

Base class for all syntax tree nodes with the following node types:

```python
class SyntaxTreeNode:
    def __init__(self, node_type: str, value=None, left=None, right=None,
                 third_child=None, args=None, line_number=0):
        self.node_type = node_type      # 'value', 'function', 'binary_op', etc.
        self.value = value              # Variable name / function name / operator / type
        self.left = left                # Left child (for binary/unary/ternary)
        self.right = right              # Right child (for binary/ternary true expr)
        self.third_child = third_child  # Third child (for ternary false expr)
        self.args = args if args else []  # Function arguments
        self.line_number = line_number
```

| Node Type | Description | Children |
|-----------|-------------|----------|
| `value` | Variable/constant values | None |
| `function` | Function calls | `args` list |
| `binary_op` | Binary operations (+, -, *, /, ., etc.) | `left`, `right` |
| `unary_op` | Unary operations (-, !) | `left` |
| `cast` | Type casting ((float3x3)expr) | `left` |
| `ternary` | Conditional (a ? b : c) | `left` (cond), `right` (true), `third_child` (false) |

### 2. SyntaxTreeParser (hlsl_syntax_tree.py:174-288)

Parses HLSL expressions into syntax trees with proper operator precedence.

**Operator Precedence (low to high)**:
```python
_OPERATORS = {
    '||': 1, '&&': 2,
    '==': 3, '!=': 3,
    '<': 4, '>': 4, '<=': 4, '>=': 4,
    '+': 5, '-': 5,
    '*': 6, '/': 6,
}
```

Special operators:
- `.` (swizzle): Highest precedence, handled separately
- Ternary `?:`: Handled via `_find_ternary_colon`

**Parsing Stages**:
1. Type casts: `(float3x3)expr`
2. Parenthesized expressions: `(expr)`
3. Ternary operator: `a ? b : c`
4. Binary operators with right-to-left associativity
5. Function calls: `float2/3/4(...)`, `mul(...)`, `transpose(...)`
6. Variable/constant values

**Caching Functions** (with `@lru_cache(maxsize=256)`):
- `_split_args_cached()`: Split function arguments
- `_find_top_level_operator_cached()`: Find lowest-precedence operator
- `_is_proper_paren()`: Check parenthesis validity
- `_find_ternary_colon()`: Find ternary colon position

### 3. HLSLInterpreter (hlsl_interpreter.py:241-2313)

Main interpreter class with key components:

**Data Structures**:
```python
@dataclass
class FieldDefinition:
    field_type: str      # float3, float4x4, etc.
    name: str           # Variable name
    semantic: str       # POSITION, NORMAL, COLOR, etc.
    data: List[Any] = None

@dataclass
class StructDefinition:
    name: str
    fields: List[FieldDefinition]

@dataclass
class CbufferDefinition:
    name: str
    fields: List[FieldDefinition]

@dataclass
class Vertex:
    index: int
    input_data: Dict[str, Any]
    output_data: Dict[str, Any]
    input_position/output_position: List[float]
    input_normal/output_normal: List[float]
    input_color/output_color: List[float]
    input_texcoord/output_texcoord: List[float]
```

**Key Methods**:
| Method | Description |
|--------|-------------|
| `interpret(hlsl_file_path, csv_folder_path)` | Parse HLSL, load CSV data |
| `executeVS(main_func, vs_input, code, execute_count)` | Execute vertex shader |
| `evaluate_expression(expr, local_vars)` | Evaluate expression via syntax tree |
| `evaluate_syntax_tree(node, local_vars)` | Execute syntax tree node |
| `execute_function_node(node, local_vars)` | Handle function calls |
| `get_value(name, local_vars)` | Retrieve variable with swizzle support |
| `compare_vs_output_with_golden(...)` | Validate against golden data |

---

## Data Types

```python
DATA_TYPE_LIST = [
    'float4x4', 'float3x3',  # Matrices (64 bytes, 36 bytes)
    'float4', 'float3', 'float2', 'float',  # Float vectors/scalar
    'uint4', 'uint3', 'uint2', 'uint',  # Unsigned integers
    'int4', 'int3', 'int2', 'int',  # Signed integers
    'bool'  # Boolean
]

_TYPE_SIZE_MAP = {
    'float4x4': 64, 'float3x3': 36, 'float4': 16, 'float3': 12,
    'float2': 8, 'float': 4, 'uint4': 16, 'uint3': 12, 'uint2': 8,
    'uint': 4, 'int4': 16, 'int3': 12, 'int2': 8, 'int': 4, 'bool': 4
}
```

### Type Conversion Rules

| Source | Target | Conversion |
|--------|--------|------------|
| float | float2/float3/float4 | Replicate to vector |
| float3x3 | float4x4 | Pad to 4x4 (bottom-right = 1) |
| float4x4 | float3x3 | Extract upper-left 3x3 |
| float4x4/float3x3 | float2x2 | Extract upper-left 2x2 |

---

## Supported Built-in Functions

| Function | Args | Description |
|----------|------|-------------|
| `transpose` | 1 (matrix) | Swap rows and columns |
| `normalize` | 1 (vector) | Scale to unit length |
| `length` | 1 (vector) | Euclidean length |
| `dot` | 2 (vectors) | Sum of element products |
| `cross` | 2 (float3) | Vector cross product |
| `reflect` | 2 (vectors) | Reflect I around N: R = I - 2*N*(N·I) |
| `mul` | 2 | Matrix-vector, vector-matrix, or matrix-matrix multiplication |
| `pow` | 2 | Exponentiation (handles base=0 to avoid INF) |
| `max` / `min` | 2 | Element-wise clamp |
| `abs` | 1 | Absolute value |
| `sin` / `cos` | 1 | Trigonometry |

### Vector Constructors

```python
float2(x, y)
float3(x, y, z)
float4(x, y, z, w)
```

---

## Supported Operators

### Arithmetic Operators
| Operator | Description |
|----------|-------------|
| `+` | Addition (scalar/vector/matrix) |
| `-` | Subtraction (scalar/vector) |
| `*` | Multiplication (scalar/vector) |
| `/` | Division (scalar/vector) |
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
```

---

## Development Timeline

### Phase 1: Foundation (Commits ~1-20)

| Feature | Description |
|---------|-------------|
| Initial code | Basic HLSL execution framework with struct/cbuffer parsing |
| Struct/cbuffer refinements | Improved parsing of HLSL structs |
| CSV data loading | Load struct/cbuffer data from CSV files |
| executeVS framework | Basic vertex shader execution |
| JSON configuration | Load settings from JSON config file |
| Syntax tree creation | Initial expression parsing into tree |

### Phase 2: Expression Evaluation (Commits ~21-40)

| Feature | Description |
|---------|-------------|
| execute_function refinement | Improved function execution and evaluation printing |
| Syntax tree module | Proper operator precedence implementation |
| Execution printing | Syntax tree operation debugging output |
| Matrix-vector multiplication | Column-major matrix handling, float3x3 cast support |
| IEEE754 float precision | Handling float precision in Python |

### Phase 3: Operators & Functions (Commits ~41-60)

| Feature | Description |
|---------|-------------|
| **Dot operator** | Vector element access (e.g., `Attenuation.x`) |
| Swizzle support | xyzw and rgba notation for vector components |
| Comments and casts | Float4x4/float3x3 cast, golden comparison output |
| Argument validation | Fixed max/min/dot argument count validation |
| All expressions via syntax tree | Fixed operator precedence |

### Phase 4: Control Flow & Performance (Commits ~61-80)

| Feature | Description |
|---------|-------------|
| Print controller | Sequence-based logging control |
| JSON config | Timing instrumentation via config |
| Function body parsing | Fixed parsing of multi-statement function bodies |
| Two-char operator fix | Proper handling of `<=`, `>=` operators |
| **Function cache** | Performance optimization to avoid re-parsing |

### Phase 5: Advanced Features (Commits ~81-107)

| Feature | Description |
|---------|-------------|
| VS execution count | Per-vertex execution tracking |
| **If-cond-else support** | Conditional execution in shaders |
| Output swizzle fix | Fixed `output.Color.xxx` syntax |
| **Thread workers** | Multi-threaded execution via ThreadPoolExecutor |
| **MeshView GUI** | Dual-window visualization (input/output) |
| Animation rendering | Vertex-by-vertex animation playback |

---

## Key Technical Decisions

### 1. Row-Vector Math

HLSL uses row-vector notation, so the interpreter multiplies matrix by vector:
```python
def mul_matrix_vector(self, m: List[List[float]], v: List[float]) -> List[float]:
    if not v or any(x is None for x in v):
        return [0, 0, 0, 0]
    if not m:
        return [0, 0, 0, 0]
    return [sum(v[i] * m[i][j] for i in range(len(v))) for j in range(len(m[0]))]
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
```

Golden output comparison with configurable tolerance to handle floating-point precision issues.

### 7. Log Caching

```python
self._log_cache = []
self._log_cache_size = 10 * 1024 * 1024  # 10MB cache

def log_output(self, *args, **kwargs):
    msg = ' '.join(str(arg) for arg in args)
    if self._log_cache_bytes + len(msg_bytes) >= self._log_cache_size:
        self._flush_log_cache()
    self._log_cache.append(msg + '\n')
    self._log_cache_bytes += len(msg_bytes)
```

Buffers log messages and flushes to file when cache is full.

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

### JSON Configuration

```json
{
    "print_sequence": 100,
    "max_workers": 4,
    "tolerance": 1e-6,
    "log_file": true,
    "log_level": "INFO",
    "printSyntaxTree": true,
    "primitive_topology": 4
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
| Vertex picking | Select vertices from input/output windows |

### Supported Primitive Topologies

| Topology | Value | Description |
|----------|-------|-------------|
| POINTLIST | 1 | Points only |
| LINELIST | 2 | Line pairs |
| LINESTRIP | 3 | Connected lines |
| TRIANGLELIST | 4 | Individual triangles |
| TRIANGLESTRIP | 5 | Connected triangles |
| TRIANGLEFAN | 6 | Triangle fan (deprecated in D3D11) |

---

## Test Cases

The interpreter includes multiple test cases for validation:

| Test Case | Description |
|-----------|-------------|
| `color-correct-ninjia-of-collision` | Correct color output with collision detection |
| `color-correct-ninjia-of-collision-if-cond` | With conditional statements |
| `color-correct-ninjia-of-collision-if-cond-multi-stmts` | Multiple statements in if blocks |
| `specular_too_shining` | Specular lighting bug case (RdotV=0 causes pow to Inf) |
| `wrong_constant_attenuation` | Light attenuation bug case |
| `constant_buffer_attenuation_wrong` | Constant buffer data issue |

---

## Bug Fixes

### 1. Specular Too Shining (commit 7792e8a, f0f5f72)
- **Issue**: `pow(base, exp)` where `base=0` causes INF
- **Fix**: Check if RdotV is zero before calling pow

### 2. Operator Precedence (commit d0da5b0)
- **Issue**: Expressions like `1.0 / (a + b * c)` were parsed incorrectly
- **Fix**: Proper binary operator precedence with right-to-left associativity

### 3. Two-Character Operators (commit ad6d8c2)
- **Issue**: `<=`, `>=`, `==`, `!=` not handled correctly
- **Fix**: Check for two-character operators before single-char operators

### 4. Float Number's Dot (commit 275d110)
- **Issue**: Float numbers like `1.0` treated dot as swizzle operator
- **Fix**: Don't treat float number's dot as dot operator

### 5. Vector Matrix Multiplication (commit d7b56fc)
- **Issue**: Vector multiplied by matrix incorrectly
- **Fix**: Use column-major matrix multiplication

### 6. Struct Member Access (commit 4a12d93)
- **Issue**: Dot operator used on struct members incorrectly
- **Fix**: Dot operator only for vector swizzle, not struct member

### 7. If-Condition-Else (commit dbc4c87, 6f981a0)
- **Issue**: Multiple statements in if blocks not executing correctly
- **Fix**: Merge if/else statements before executing

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

---

## Statistics

| Metric | Value |
|--------|-------|
| Total commits (all projects) | 301 |
| hlsl-inter commits | 120 |
| Effective hlsl-inter commits | 107 |
| Main interpreter lines | ~2313 |
| Syntax tree parser lines | ~288 |
| MeshView GUI lines | ~1549 |
| Test cases | 6+ |
| Supported data types | 14 |
| Built-in functions | 12+ |