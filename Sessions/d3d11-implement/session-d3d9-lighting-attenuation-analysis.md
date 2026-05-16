# D3D9 SetLight衰减参数工作原理

**Session ID:** ses_1d3d7c1bcffeZaQXP4p83I80Nq
**Created:** 2026/5/15 23:01:35
**Updated:** 2026/5/15 23:01:45

---

## User

d3d9 SetLight函数送入的Attenuation参数，是如何影响光照的？

---

## Assistant (Build · MiniMax-M2.7 · 9.5s)

_Thinking:_

The user is asking about how the Attenuation parameter in the SetLight function works in D3D9 (Direct3D 9). This is a graphics programming question about the Direct3D 9 lighting model.

In Direct3D 9, when you call SetLight to set a light, you pass a D3DLIGHT9 structure which contains attenuation parameters. The attenuation defines how the light intensity decreases over distance.

The D3D9 light attenuation formula is:
Attenuation = 1 / (A0 + A1 * d + A2 * d^2)

Where:
- A0 = constant attenuation (attenuation[0])
- A1 = linear attenuation (attenuation[1])
- A2 = quadratic attenuation (attenuation[2])
- d = distance from the light to the vertex/fragment

This is standard for point lights and spot lights. For directional lights, attenuation is not used (or is always 1).

Let me provide a concise answer about this.



在 Direct3D 9 中，`SetLight` 设置的光源衰减公式为：

```
Attenuation = 1 / (A0 + A1 × d + A2 × d²)
```

其中：
- **A0** (attenuation[0])：常数衰减
- **A1** (attenuation[1])：线性衰减  
- **A2** (attenuation[2])：二次衰减
- **d**：顶点到光源的距离

**效果：**
- `A0=1, A1=0, A2=0` → 无衰减，恒定亮度
- `A0=0, A1=1, A2=0` → 线性衰减 (1/d)
- `A0=0, A1=0, A2=1` → 二次衰减 (1/d²) — 最物理真实
- 通常使用 `A0=0, A1=0.1, A2=0` 的组合

D3D9 自动将这个衰减系数乘以光源的 `Diffuse`/`Specular` 颜色值，最终影响光照计算结果。

---

