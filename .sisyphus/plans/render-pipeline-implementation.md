# 1️⃣ Overview

| Item | Description |
|------|-------------|
| **Goal** | Deliver a fully‑fledged, modular 3‑D rendering pipeline that adheres to the pre‑approved design. |
| **Scope** | Core engine (`source/Irrlicht/`), platform drivers, build system, unit & integration tests. |
| **Architecture** | 1. **Device abstraction** (`CIrrDevice*`) – creates windows, selects `IVideoDriver`. <br> 2. **Driver interface** (`IVideoDriver`) – OpenGL, DirectX, software renderer implementations. <br> 3. **Scene graph** (`ISceneManager`, `ISceneNode`) – hierarchical culling, traversal. <br> 4. **Geometry & materials** (`IMeshBuffer`, `SMaterial`, `ITexture`, `IMaterialRenderer*`). <br> 5. **Resource pools** – GPU buffers, textures, shaders. |
| **Key Decisions** | - No STL/RTTI/exceptions (manual ref counting). <br> - Custom containers (`core::array`, `core::stringc`). <br> - Shader caching and material renderer registry. <br> - Deferred batch rendering to minimize state changes. |
| **Deliverables** | 1. Complete driver API & two drivers (OpenGL + software). <br> 2. Full scene‑graph implementation with culling. <br> 3. Mesh, texture, shader systems. <br> 4. Test harness & CI pipeline. |

# 2️⃣ Parallel Task Architecture

| Sprint | Parallel Work Units | Lead | Tools |
|--------|-----|------|-------|
| **0 – Foundation** | Project layout, CI templates. | Build/CI | `git`, `cmake` |
| **1 – Core Device API** | `IrrlichtDevice`, `IVideoDriver`, `CIrrDeviceStub`. | Core | `cscope`, `grep` |
| **2 – Concrete Drivers** | OpenGL driver, software rasterizer. | Platform | `OpenGL`, `SDL` |
| **3 – Resource Management** | GPU buffer pool, texture loader, shader objects. | Rendering | `glslang` |
| **4 – Scene Graph** | `ISceneNode`, camera, frustum. | Scene | `glm` |
| **5 – Mesh System** | `IMeshBuffer`, cache, vertex formats. | Geometry | `assimp` |
| **6 – Rendering Pipeline** | Deferred draw‑call collector, material sorting, post‑process shaders. | Pipeline | `OGLPP` |
| **7 – Testing & Optimization** | Unit tests, integration harness, profiling. | QA | `googletest`, `gperftools` |
| **8 – Polish & Docs** | API docs, examples, release candidate. | Docs | `doxygen` |

*All units within a sprint can run in parallel; cross‑sprint dependencies enforce linear progress.*

# 3️⃣ Verification & QA Strategy

| Stage | Test Type | Tool | Pass Criteria |
|-------|-----------|------|---------------|
| **Unit** | Class/module tests | `googletest` | ≥95 % coverage, no crashes |
| **Integration** | Driver switching & scene rendering | `tests/engine_fixtures.cpp` | Correct pixel dump; frame rate ≥ 60 fps on baseline hardware |
| **Cross‑Platform** | Build & smoke test | CI matrix (`.github/workflows/ci.yml`) | Build & run on Linux, macOS, Windows |
| **Performance** | FPS & memory profiling | `gperftools`, `valgrind` | Frame time ≤ 16 ms, no leaks |
| **CI** | Pull‑request validation | GitHub Actions | All tests pass, linting OK |

- **Verification checkpoints** after each sprint: CI badge + code‑review approvals. <br> - **Regression suite** resides in `tests/regress/`.

# 4️⃣ Commit & Branching Strategy

| Branch | Purpose |
|--------|---------|
| `master` | Production release |
| `dev` | Integration base |
| `feature/driver-opengl` | OpenGL driver branch |
| `feature/scene-graph` | Scene‑graph branch |
| … | Other feature branches prefixed with `feature/*` |

**Commit cadence** 
1. Small, atomic commits (≤ 3 files). 
2. Commit message: clear purpose (“Refactor IMeshBuffer to use GPU pool”). 
3. Include unit tests in same commit. 
4. Rebase onto `dev` before PR. 
5. 2‑approval code‑review required.

**Pull‑request template**

```
## Summary
- Feature: ___
- Scope: ___

## Files touched
- ...

## Tests
- Added: ...

## Notes
- ...
```

# 5️⃣ Risk & Mitigation

| Risk | Mitigation |
|------|------------|
| Ref‑counting bugs | Use atomic reference counters; exhaustive cross‑platform tests. |
| GL init failure on legacy GPUs | Provide a software fallback and detect GL version at runtime. |
| Scene traversal bugs | Unit‑test static and dynamic scene trees; use deterministic scene graphs in tests. |
| Build divergence per OS | Automate with `CMake` and platform detection; run tests on CI for each OS. |
| Performance regressions | Include baseline metrics in sprint 7; automate performance regression tests. |

# 6️⃣ Milestones & Deliverables

| Milestone | Estimated Date | Deliverable |
|-----------|----------------|-------------|
| Sprint 0 | Week 2 | Build scaffolding, CI stub |
| Sprint 2 | Week 6 | Two working drivers |
| Sprint 5 | Week 12 | Functional scene‑graph |
| Sprint 7 | Week 14 | Full pipeline + test suite |
| Release Candidate | Week 16 | Production build in `release/` |

# 7️⃣ Deliverable File Path

**Copy the full content above to:**
`.sisyphus/plans/render-pipeline-implementation.md`

---

The plan above respects the “do not modify source files” constraint while providing a complete, decision‑complete roadmap for the implementation team.