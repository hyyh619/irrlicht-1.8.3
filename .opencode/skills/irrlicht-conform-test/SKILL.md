---
name: irrlicht-conform-test
description: |
  Irrlicht Engine conformance test skill - automatically builds the project, runs tests, and compares screenshots with golden references.
  Use this skill whenever the user wants to:
  - Build and test the Irrlicht Engine project
  - Run conformance tests and verify screenshot outputs match golden references
  - Check if any tests fail after building with CONFORM_TEST enabled
  - Run regression tests for the graphics engine
---

## Test List
The following examples are tested:
1. 01.HelloWorld
2. 02.Quake3Map

## Workflow

### Step 1: Build the Project

Build the entire project using the build script:
```bash
cd examples && build_debug_x64_conform.bat
```

This compiles with CONFORM_TEST enabled, which makes test programs capture screenshots automatically.

### Step 2: Run Each Test

For each example in the test list:
1. Navigate to the built executable location
2. Run the example to generate screenshot.bmp

The executable is located at: `bin/Win64-VisualStudio/<ExampleName>.exe`

### Step 3: Compare Screenshots

For each test, compare the generated screenshot with its golden reference:
```bash
python Scripts/compare_screenshots.py <ExampleName>
```

Golden images are stored at: `examples/<ExampleName>/Frame1-screenshot-golden.bmp`

### Step 4: Report Results

After all tests complete, summarize the results:
- PASS: All pixels match (or differences within tolerance)
- FAIL: Too many pixel differences

For each test example, report:
- Example name
- PASS or FAIL
- Number of different pixels (if any)