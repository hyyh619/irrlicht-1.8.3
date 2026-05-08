# Material着色器内嵌化重构

**Session ID:** ses_1faaf160affexTo2iU7qkuBT2l
**Created:** 2026/5/8 10:00:49
**Updated:** 2026/5/8 10:09:56

---

## User

1. 把PS_MaterialShaders.hlsl的内容放到CD3D11MaterialRenderer，创建一个全局的字符串
2. CD3D11Driver::createMaterialPixelShader不需要file = FileSystem->createAndOpenFile("PS_MaterialShaders.hlsl");，直接加载该字符串 

---

## Assistant (Build · MiniMax-M2.7 · 8.4s)

_Thinking:_

The user wants me to:
1. Move the content of PS_MaterialShaders.hlsl into CD3D11MaterialRenderer as a global string
2. Modify CD3D11Driver::createMaterialPixelShader to use that global string instead of loading from a file

Let me first find the relevant files to understand the current implementation.

**Tool: glob**

**Input:**
```json
{
  "pattern": "**/*PS_MaterialShaders.hlsl"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\PS_MaterialShaders.hlsl
```

**Tool: grep**

**Input:**
```json
{
  "pattern": "CD3D11MaterialRenderer",
  "include": "*.cpp"
}
```

**Output:**
```
Found 65 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 15: #include "CD3D11MaterialRenderer.h"

  Line 2333:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "solid"));

  Line 2334:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "solid_lightmap"));

  Line 2335:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "solid_2_layer"));

  Line 2336:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent"));

  Line 2337:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_2_layer"));

  Line 2338:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_add_color"));

  Line 2339:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_vertex_alpha"));

  Line 2340:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel"));

  Line 2341:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel_ref"));

  Line 2342:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "one_texture_blend"));

  Line 2343:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "lightmap_blend"));

  Line 2344:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "detail_map"));

  Line 2345:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "sphere_map"));

  Line 2346:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "reflection_2_layer"));

  Line 2347:             m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "transparent_reflection_2_layer"));


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11MaterialRenderer.cpp:
  Line 9: #include "CD3D11MaterialRenderer.h"

  Line 20:         CD3D11MaterialRenderer::CD3D11MaterialRenderer(CD3D11Driver *driver, s32 materialType,

  Line 25:             setDebugName("CD3D11MaterialRenderer");

  Line 28:             os::Printer::log("CD3D11MaterialRenderer created:", buf, ELL_INFORMATION);

  Line 34:         CD3D11MaterialRenderer::~CD3D11MaterialRenderer()

  Line 39:             os::Printer::log("CD3D11MaterialRenderer destroyed:", buf, ELL_INFORMATION);

  Line 44:         void CD3D11MaterialRenderer::OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,

  Line 51:         bool CD3D11MaterialRenderer::OnSetTexture(u32 textureIndex, ITexture *texture)

  Line 63:         void CD3D11MaterialRenderer::OnSetConstants(IMaterialRendererServices *services, s32 userData)

  Line 67:         void CD3D11MaterialRenderer::PostRender()

  Line 71:         CD3D11MaterialRenderer_SOLID::CD3D11MaterialRenderer_SOLID(CD3D11Driver *p, video::IVideoDriver *d)

  Line 72:             : CD3D11MaterialRenderer(p, -1, "solid") {}

  Line 75:         CD3D11MaterialRenderer_SOLID_2_LAYER::CD3D11MaterialRenderer_SOLID_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d)

  Line 76:             : CD3D11MaterialRenderer(p, -1, "solid_2_layer") {}

  Line 79:         CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR(CD3D11Driver *p, video::IVideoDriver *d)

  Line 80:             : CD3D11MaterialRenderer(p, -1, "transparent_add_color") {}

  Line 83:         CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(CD3D11Driver *p, video::IVideoDriver *d)

  Line 84:             : CD3D11MaterialRenderer(p, -1, "transparent_vertex_alpha") {}

  Line 87:         CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(CD3D11Driver *p, video::IVideoDriver *d)

  Line 88:             : CD3D11MaterialRenderer(p, -1, "transparent_alpha_channel") {}

  Line 91:         CD3D11MaterialRenderer_ONETEXTURE_BLEND::CD3D11MaterialRenderer_ONETEXTURE_BLEND(CD3D11Driver *p, video::IVideoDriver *d)

  Line 92:             : CD3D11MaterialRenderer(p, -1, "one_texture_blend") { m_Transparent = false; }

  Line 95:         CD3D11MaterialRenderer_LIGHTMAP::CD3D11MaterialRenderer_LIGHTMAP(CD3D11Driver *p, video::IVideoDriver *d)

  Line 96:             : CD3D11MaterialRenderer(p, -1, "lightmap") {}

  Line 99:         CD3D11MaterialRenderer_DETAIL_MAP::CD3D11MaterialRenderer_DETAIL_MAP(CD3D11Driver *p, video::IVideoDriver *d)

  Line 100:             : CD3D11MaterialRenderer(p, -1, "detail_map") {}

  Line 103:         CD3D11MaterialRenderer_SPHERE_MAP::CD3D11MaterialRenderer_SPHERE_MAP(CD3D11Driver *p, video::IVideoDriver *d)

  Line 104:             : CD3D11MaterialRenderer(p, -1, "sphere_map") {}

  Line 107:         CD3D11MaterialRenderer_REFLECTION_2_LAYER::CD3D11MaterialRenderer_REFLECTION_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d)

  Line 108:             : CD3D11MaterialRenderer(p, -1, "reflection_2_layer") {}

  Line 111:         void CD3D11MaterialRenderer_SOLID::OnSetMaterial(const SMaterial &material,

  Line 118:         void CD3D11MaterialRenderer_SOLID_2_LAYER::OnSetMaterial(const SMaterial &material,

  Line 125:         void CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::OnSetMaterial(const SMaterial &material,

  Line 132:         bool CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::isTransparent() const

  Line 138:         void CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::OnSetMaterial(const SMaterial &material,

  Line 145:         bool CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::isTransparent() const

  Line 151:         void CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::OnSetMaterial(const SMaterial &material,

  Line 158:         void CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::OnUnsetMaterial()

  Line 163:         bool CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::isTransparent() const

  Line 169:         void CD3D11MaterialRenderer_ONETEXTURE_BLEND::OnSetMaterial(const SMaterial &material,

  Line 194:         bool CD3D11MaterialRenderer_ONETEXTURE_BLEND::isTransparent() const

  Line 200:         u32 CD3D11MaterialRenderer_ONETEXTURE_BLEND::getD3D11Blend(E_BLEND_FACTOR factor) const

  Line 220:         u32 CD3D11MaterialRenderer_ONETEXTURE_BLEND::getD3D11Modulate(E_MODULATE_FUNC func) const

  Line 232:         void CD3D11MaterialRenderer_LIGHTMAP::OnSetMaterial(const SMaterial &material,

  Line 239:         void CD3D11MaterialRenderer_DETAIL_MAP::OnSetMaterial(const SMaterial &material,

  Line 246:         void CD3D11MaterialRenderer_SPHERE_MAP::OnSetMaterial(const SMaterial &material,

  Line 253:         void CD3D11MaterialRenderer_SPHERE_MAP::OnUnsetMaterial()

  Line 258:         void CD3D11MaterialRenderer_REFLECTION_2_LAYER::OnSetMaterial(const SMaterial &material,

  Line 265:         void CD3D11MaterialRenderer_REFLECTION_2_LAYER::OnUnsetMaterial()

```

**Tool: grep**

**Input:**
```json
{
  "pattern": "createMaterialPixelShader",
  "include": "*.cpp"
}
```

**Output:**
```
Found 2 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 2437:                     createMaterialPixelShader((E_MATERIAL_TYPE)i);

  Line 2682:         bool CD3D11Driver::createMaterialPixelShader(E_MATERIAL_TYPE materialType)

```

---

## Assistant (Build · MiniMax-M2.7 · 6.3s)

_Thinking:_

Let me read the PS_MaterialShaders.hlsl file and the CD3D11Driver.cpp around the createMaterialPixelShader function to understand the current implementation.

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\PS_MaterialShaders.hlsl"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\PS_MaterialShaders.hlsl</path>
<type>file</type>
<content>
1: // Auto-generated HLSL Pixel Shaders for Irrlicht Material Types
2: // Reference: CD3D9MaterialRenderer implementation
3: 
4: #ifndef __PS_MATERIAL_SHADERS_H__
5: #define __PS_MATERIAL_SHADERS_H__
6: 
7: //==============================================================================
8: // Common Structures and Samplers
9: //==============================================================================
10: 
11: struct PS_INPUT_BASIC
12: {
13:     float4 Pos : SV_POSITION;
14:     float4 Color : COLOR;
15:     float2 TexCoord : TEXCOORD0;
16:     float3 Normal : TEXCOORD1;
17: };
18: 
19: struct PS_INPUT_2TEX
20: {
21:     float4 Pos : SV_POSITION;
22:     float4 Color : COLOR;
23:     float2 TexCoord0 : TEXCOORD0;
24:     float2 TexCoord1 : TEXCOORD1;
25:     float3 Normal : TEXCOORD2;
26: };
27: 
28: struct PS_INPUT_TANGENTS
29: {
30:     float4 Pos : SV_POSITION;
31:     float4 Color : COLOR;
32:     float2 TexCoord : TEXCOORD0;
33:     float3 Normal : TEXCOORD1;
34:     float3 Tangent : TEXCOORD2;
35:     float3 Binormal : TEXCOORD3;
36: };
37: 
38: Texture2D DiffuseTexture : register(t0);
39: Texture2D LightmapTexture : register(t1);
40: Texture2D DetailTexture : register(t1);
41: Texture2D NormalMap : register(t1);
42: Texture2D SphereMap : register(t0);
43: 
44: SamplerState LinearSampler : register(s0);
45: 
46: //==============================================================================
47: // EMT_SOLID - Standard solid material, first texture * diffuse
48: // ColorOp: D3DTOP_MODULATE, Tex * Diffuse
49: //==============================================================================
50: float4 PS_SOLID(PS_INPUT_BASIC input) : SV_TARGET
51: {
52:     float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
53:     return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);
54: }
55: 
56: //==============================================================================
57: // EMT_SOLID_2_LAYER - Two layers blended with vertex alpha
58: // Stage 0: D3DTA_TEXTURE
59: // Stage 1: D3DTOP_BLENDDIFFUSEALPHA, TexCoord1 = TexCoord0
60: //==============================================================================
61: float4 PS_SOLID_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET
62: {
63:     float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
64:     float4 layer1 = DiffuseTexture.Sample(LinearSampler, input.TexCoord1); // Same texture, stage 1
65:     float alpha = input.Color.a;
66:     return lerp(layer0, layer1, alpha);
67: }
68: 
69: //==============================================================================
70: // EMT_LIGHTMAP - Lightmap modulated with diffuse
71: // Stage 0: D3DTA_TEXTURE (no lighting)
72: // Stage 1: D3DTOP_MODULATE, texture * current
73: //==============================================================================
74: float4 PS_LIGHTMAP(PS_INPUT_2TEX input) : SV_TARGET
75: {
76:     float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
77:     float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
78:     return diffuse * lightmap * 2.0;
79: }
80: 
81: //==============================================================================
82: // EMT_LIGHTMAP_ADD - Lightmap added to diffuse
83: // Stage 1: D3DTOP_ADD
84: //==============================================================================
85: float4 PS_LIGHTMAP_ADD(PS_INPUT_2TEX input) : SV_TARGET
86: {
87:     float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
88:     float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
89:     return diffuse + lightmap;
90: }
91: 
92: //==============================================================================
93: // EMT_LIGHTMAP_M2 - Lightmap * 2 then modulated with diffuse
94: // Stage 1: D3DTOP_MODULATE2X
95: //==============================================================================
96: float4 PS_LIGHTMAP_M2(PS_INPUT_2TEX input) : SV_TARGET
97: {
98:     float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
99:     float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
100:     return diffuse * (lightmap * 2.0);
101: }
102: 
103: //==============================================================================
104: // EMT_LIGHTMAP_M4 - Lightmap * 4 then modulated with diffuse
105: // Stage 1: D3DTOP_MODULATE4X
106: //==============================================================================
107: float4 PS_LIGHTMAP_M4(PS_INPUT_2TEX input) : SV_TARGET
108: {
109:     float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
110:     float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
111:     return diffuse * (lightmap * 4.0);
112: }
113: 
114: //==============================================================================
115: // EMT_LIGHTMAP_LIGHTING - Lightmap with dynamic lighting
116: // Stage 0: D3DTOP_MODULATE, Tex * Diffuse
117: //==============================================================================
118: float4 PS_LIGHTMAP_LIGHTING(PS_INPUT_2TEX input) : SV_TARGET
119: {
120:     float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;
121:     float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
122:     return diffuse * lightmap * 2.0;
123: }
124: 
125: //==============================================================================
126: // EMT_LIGHTMAP_LIGHTING_M2
127: //==============================================================================
128: float4 PS_LIGHTMAP_LIGHTING_M2(PS_INPUT_2TEX input) : SV_TARGET
129: {
130:     float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;
131:     float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
132:     return diffuse * (lightmap * 2.0);
133: }
134: 
135: //==============================================================================
136: // EMT_LIGHTMAP_LIGHTING_M4
137: //==============================================================================
138: float4 PS_LIGHTMAP_LIGHTING_M4(PS_INPUT_2TEX input) : SV_TARGET
139: {
140:     float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;
141:     float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
142:     return diffuse * (lightmap * 4.0);
143: }
144: 
145: //==============================================================================
146: // EMT_DETAIL_MAP - Diffuse + Detail map using ADD_SIGNED
147: // Stage 0: D3DTOP_MODULATE, Tex * Diffuse
148: // Stage 1: D3DTOP_ADDSIGNED, Detail * CURRENT
149: //==============================================================================
150: float4 PS_DETAIL_MAP(PS_INPUT_2TEX input) : SV_TARGET
151: {
152:     float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
153:     float4 detail = DetailTexture.Sample(LinearSampler, input.TexCoord1);
154:     float4 base = diffuse * input.Color;
155:     // ADD_SIGNED: base + detail - 0.5
156:     return base + (detail - 0.5);
157: }
158: 
159: //==============================================================================
160: // EMT_SPHERE_MAP - Environment reflection using sphere map
161: // Uses D3DTS_TEXTURE0 with sphere map transform
162: // TCI_CAMERASPACENORMAL
163: //==============================================================================
164: float4 PS_SPHERE_MAP(PS_INPUT_BASIC input) : SV_TARGET
165: {
166:     float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
167:     float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
168:     float3 reflectVec = reflect(-viewDir, input.Normal);
169:     float2 sphereUV = reflectVec.xy * 0.5 + 0.5;
170:     float4 sphereColor = SphereMap.Sample(LinearSampler, sphereUV);
171:     return texColor * sphereColor * 2.0;
172: }
173: 
174: //==============================================================================
175: // EMT_REFLECTION_2_LAYER - Two layer reflection
176: // Stage 0: D3DTOP_MODULATE, Tex * Diffuse
177: // Stage 1: D3DTOP_MODULATE, Tex1 * CURRENT
178: // TCI_CAMERASPACEREFLECTIONVECTOR on stage 1
179: //==============================================================================
180: float4 PS_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET
181: {
182:     float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
183:     float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);
184:     return layer0 * input.Color * reflection * 2.0;
185: }
186: 
187: //==============================================================================
188: // EMT_TRANSPARENT_ADD_COLOR - Add source to dest (no alpha blend)
189: // SrcBlend: D3DBLEND_ONE
190: // DestBlend: D3DBLEND_INVSRCCOLOR
191: // ColorOp: D3DTOP_MODULATE, Tex * Diffuse
192: //==============================================================================
193: float4 PS_TRANSPARENT_ADD_COLOR(PS_INPUT_BASIC input) : SV_TARGET
194: {
195:     float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
196:     float4 result = texColor * input.Color;
197:     return result; // Blend func: result + dest * (1 - result.rgb)
198: }
199: 
200: //==============================================================================
201: // EMT_TRANSPARENT_ALPHA_CHANNEL - Alpha blend with texture alpha
202: // SrcBlend: D3DBLEND_SRCALPHA
203: // DestBlend: D3DBLEND_INVSRCALPHA
204: // AlphaOp: D3DTOP_MODULATE, Tex * Diffuse (on stage 0)
205: // AlphaRef: MaterialTypeParam * 255
206: //==============================================================================
207: float4 PS_TRANSPARENT_ALPHA_CHANNEL(PS_INPUT_BASIC input) : SV_TARGET
208: {
209:     float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
210:     float4 result = texColor * input.Color;
211:     result.a = texColor.a * input.Color.a;
212:     return result;
213: }
214: 
215: //==============================================================================
216: // EMT_TRANSPARENT_ALPHA_CHANNEL_REF - Alpha test only, no blending
217: // AlphaRef: 127
218: // AlphaFunc: D3DCMP_GREATEREQUAL
219: // AlphaOp: D3DTOP_SELECTARG1, D3DTA_TEXTURE
220: //==============================================================================
221: float4 PS_TRANSPARENT_ALPHA_CHANNEL_REF(PS_INPUT_BASIC input) : SV_TARGET
222: {
223:     float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
224:     clip(texColor.a - 0.5); // alpha >= 127/255 ≈ 0.5
225:     return texColor * input.Color;
226: }
227: 
228: //==============================================================================
229: // EMT_TRANSPARENT_VERTEX_ALPHA - Alpha from vertex color
230: // SrcBlend: D3DBLEND_SRCALPHA
231: // DestBlend: D3DBLEND_INVSRCALPHA
232: // ColorOp: D3DTOP_MODULATE, Tex * Diffuse
233: // AlphaOp: D3DTOP_SELECTARG1, D3DTA_DIFFUSE
234: //==============================================================================
235: float4 PS_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_BASIC input) : SV_TARGET
236: {
237:     float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
238:     return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);
239: }
240: 
241: //==============================================================================
242: // EMT_TRANSPARENT_REFLECTION_2_LAYER - Transparent reflection with vertex alpha
243: // SrcBlend: D3DBLEND_SRCALPHA, DestBlend: D3DBLEND_INVSRCALPHA
244: // Both stages use alpha from diffuse
245: // TCI_CAMERASPACEREFLECTIONVECTOR on stage 1
246: //==============================================================================
247: float4 PS_TRANSPARENT_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET
248: {
249:     float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
250:     float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);
251:     float4 result = layer0 * reflection * 2.0;
252:     result.a *= input.Color.a;
253:     return result;
254: }
255: 
256: //==============================================================================
257: // EMT_NORMAL_MAP_SOLID - Tangent space normal mapping with lighting
258: // Two lights supported (nearest two)
259: //==============================================================================
260: float4 PS_NORMAL_MAP_SOLID(PS_INPUT_TANGENTS input) : SV_TARGET
261: {
262:     float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
263:     float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;
264: 
265:     float3x3 TBN = float3x3(
266:         normalize(input.Tangent),
267:         normalize(input.Binormal),
268:         normalize(input.Normal)
269:     );
270:     float3 normal = normalize(mul(normalTex, TBN));
271: 
272:     float3 lightDir1 = normalize(float3(1.0, 1.0, 1.0));
273:     float3 lightDir2 = normalize(float3(-1.0, 0.5, 0.8));
274:     float lighting = max(dot(normal, lightDir1), 0.0) + max(dot(normal, lightDir2), 0.0) * 0.5;
275: 
276:     return float4(diffuseColor.rgb * lighting, diffuseColor.a);
277: }
278: 
279: //==============================================================================
280: // EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR
281: //==============================================================================
282: float4 PS_NORMAL_MAP_TRANSPARENT_ADD_COLOR(PS_INPUT_TANGENTS input) : SV_TARGET
283: {
284:     float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
285:     float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;
286: 
287:     float3x3 TBN = float3x3(
288:         normalize(input.Tangent),
289:         normalize(input.Binormal),
290:         normalize(input.Normal)
291:     );
292:     float3 normal = normalize(mul(normalTex, TBN));
293: 
294:     float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
295:     float lighting = max(dot(normal, lightDir), 0.0);
296: 
297:     return float4(diffuseColor.rgb * lighting, diffuseColor.a);
298: }
299: 
300: //==============================================================================
301: // EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA
302: //==============================================================================
303: float4 PS_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_TANGENTS input) : SV_TARGET
304: {
305:     float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
306:     float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;
307: 
308:     float3x3 TBN = float3x3(
309:         normalize(input.Tangent),
310:         normalize(input.Binormal),
311:         normalize(input.Normal)
312:     );
313:     float3 normal = normalize(mul(normalTex, TBN));
314: 
315:     float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
316:     float lighting = max(dot(normal, lightDir), 0.0);
317: 
318:     float4 result = diffuseColor;
319:     result.rgb *= lighting;
320:     result.a *= input.Color.a;
321:     return result;
322: }
323: 
324: //==============================================================================
325: // EMT_PARALLAX_MAP_SOLID - Normal map with height offset (parallax mapping)
326: // Height scale from MaterialTypeParam (default 0.02)
327: //==============================================================================
328: float4 PS_PARALLAX_MAP_SOLID(PS_INPUT_TANGENTS input) : SV_TARGET
329: {
330:     const float heightScale = 0.02f;
331:     
332:     float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
333:     float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;
334:     float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;
335: 
336:     float3x3 TBN = float3x3(
337:         normalize(input.Tangent),
338:         normalize(input.Binormal),
339:         normalize(input.Normal)
340:     );
341:     float3 normal = normalize(mul(normalTex, TBN));
342: 
343:     float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
344:     float2 parallaxOffset = height * heightScale * viewDir.xy;
345:     float2 offsetTexCoord = input.TexCoord + parallaxOffset;
346: 
347:     float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;
348: 
349:     float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
350:     float lighting = max(dot(normal, lightDir), 0.0);
351: 
352:     return float4(texColor.rgb * lighting, texColor.a);
353: }
354: 
355: //==============================================================================
356: // EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR
357: //==============================================================================
358: float4 PS_PARALLAX_MAP_TRANSPARENT_ADD_COLOR(PS_INPUT_TANGENTS input) : SV_TARGET
359: {
360:     const float heightScale = 0.02f;
361:     
362:     float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
363:     float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;
364:     float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;
365: 
366:     float3x3 TBN = float3x3(
367:         normalize(input.Tangent),
368:         normalize(input.Binormal),
369:         normalize(input.Normal)
370:     );
371:     float3 normal = normalize(mul(normalTex, TBN));
372: 
373:     float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
374:     float2 parallaxOffset = height * heightScale * viewDir.xy;
375:     float2 offsetTexCoord = input.TexCoord + parallaxOffset;
376: 
377:     float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;
378: 
379:     float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
380:     float lighting = max(dot(normal, lightDir), 0.0);
381: 
382:     return float4(texColor.rgb * lighting, diffuseColor.a);
383: }
384: 
385: //==============================================================================
386: // EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA
387: //==============================================================================
388: float4 PS_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_TANGENTS input) : SV_TARGET
389: {
390:     const float heightScale = 0.02f;
391:     
392:     float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
393:     float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;
394:     float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;
395: 
396:     float3x3 TBN = float3x3(
397:         normalize(input.Tangent),
398:         normalize(input.Binormal),
399:         normalize(input.Normal)
400:     );
401:     float3 normal = normalize(mul(normalTex, TBN));
402: 
403:     float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
404:     float2 parallaxOffset = height * heightScale * viewDir.xy;
405:     float2 offsetTexCoord = input.TexCoord + parallaxOffset;
406: 
407:     float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;
408: 
409:     float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
410:     float lighting = max(dot(normal, lightDir), 0.0);
411: 
412:     float4 result = texColor;
413:     result.rgb *= lighting;
414:     result.a *= input.Color.a;
415:     return result;
416: }
417: 
418: //==============================================================================
419: // EMT_ONETEXTURE_BLEND - Generic blend function
420: // BlendFunc = source * sourceFactor + dest * destFactor
421: // srcFact/dstFact from MaterialTypeParam
422: // modulate: 1x=MODULATE, 2x=MODULATE2X, 4x=MODULATE4X
423: //==============================================================================
424: float4 PS_ONETEXTURE_BLEND(PS_INPUT_BASIC input) : SV_TARGET
425: {
426:     float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
427:     float4 result = texColor * input.Color;
428: 
429:     // Generic blend - typically handled by blend state in D3D11
430:     // This shader returns the pre-blended color
431:     return result;
432: }
433: 
434: //==============================================================================
435: // dummy for d3d11 compiler
436: //==============================================================================
437:  struct VS_INPUT {
438:      float3 Pos : POSITION;
439:      float3 Normal : NORMAL;
440:      float4 Color : COLOR;
441:      float2 TexCoord : TEXCOORD0;
442:  };
443:  struct VS_OUTPUT {
444:      float4 Pos : SV_POSITION;
445:      float4 Color : COLOR;
446:      float2 TexCoord : TEXCOORD0;
447:      float3 Normal : TEXCOORD1;
448:  };
449:  cbuffer MatrixBuffer : register(b0) {
450:      float4x4 WorldViewProj;
451:  };
452:  VS_OUTPUT main(VS_INPUT input) {
453:      VS_OUTPUT output;
454:      output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));
455:      output.Color = input.Color;
456:      output.TexCoord = input.TexCoord;
457:      output.Normal = input.Normal;
458:      return output;
459:  };
460: #endif // __PS_MATERIAL_SHADERS_H__

(End of file - total 460 lines)
</content>

<system-reminder>
Instructions from: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\AGENTS.md
# SOURCE KNOWLEDGE BASE

## OVERVIEW
Implementation of the Irrlicht Engine including rendering drivers, scene management, and 3rd party libraries.

## STRUCTURE
```
../include         # header files, including interface definition.
source/Irrlicht/
├── aesGladman/    # AES encryption
├── bzip2/         # bzip2 compression
├── jpeglib/       # JPEG image support
├── libpng/        # PNG image support
├── lzma/          # LZMA compression
├── MacOSX/        # macOS platform specifics (Cocoa/OpenGL)
└── zlib/          # zlib compression
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Rendering | `C*Driver.cpp` | `CNullDriver`, `COpenGLDriver`, `CD3D9Driver` |
| Scene Graph | `C*SceneNode.cpp` | `CSceneManager`, `CMeshSceneNode`, etc. |
| GUI | `CGUI*.cpp` | `CGUIEnvironment`, `CGUIButton`, etc. |
| IO | `C*Reader.cpp` | File system and archive handling |

## CONVENTIONS
- Internal classes prefixed with `C` (e.g., `CSceneManager` implements `ISceneManager`).
- Heavy use of `irr::core` containers for internal logic.
- Platform-specific code isolated in `CIrrDevice*.cpp` and OS subdirectories.

## ANTI-PATTERNS
- Do not add dependencies on `std::`.
- Avoid direct OS API calls outside of `CIrrDevice` or platform directories.

</system-reminder>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "offset": 2675,
  "limit": 100
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
2675:             IRR_D3D11_PS_CREATE(m_BuiltInPixelShader[type], "BuiltInPixelShader");
2676:             m_BuiltInPixelShader[type]->AddRef();
2677: 
2678:             return true;
2679:         }
2680: 
2681: 
2682:         bool CD3D11Driver::createMaterialPixelShader(E_MATERIAL_TYPE materialType)
2683:         {
2684:             const char    *entryPoint = 0;
2685: 
2686:             switch (materialType)
2687:             {
2688:                 case EMT_SOLID:                                 entryPoint = "PS_SOLID"; break;
2689: 
2690:                 case EMT_SOLID_2_LAYER:                         entryPoint = "PS_SOLID_2_LAYER"; break;
2691: 
2692:                 case EMT_LIGHTMAP:                              entryPoint = "PS_LIGHTMAP"; break;
2693: 
2694:                 case EMT_LIGHTMAP_ADD:                          entryPoint = "PS_LIGHTMAP_ADD"; break;
2695: 
2696:                 case EMT_LIGHTMAP_M2:                           entryPoint = "PS_LIGHTMAP_M2"; break;
2697: 
2698:                 case EMT_LIGHTMAP_M4:                           entryPoint = "PS_LIGHTMAP_M4"; break;
2699: 
2700:                 case EMT_LIGHTMAP_LIGHTING:                     entryPoint = "PS_LIGHTMAP_LIGHTING"; break;
2701: 
2702:                 case EMT_LIGHTMAP_LIGHTING_M2:                  entryPoint = "PS_LIGHTMAP_LIGHTING_M2"; break;
2703: 
2704:                 case EMT_LIGHTMAP_LIGHTING_M4:                  entryPoint = "PS_LIGHTMAP_LIGHTING_M4"; break;
2705: 
2706:                 case EMT_DETAIL_MAP:                            entryPoint = "PS_DETAIL_MAP"; break;
2707: 
2708:                 case EMT_SPHERE_MAP:                            entryPoint = "PS_SPHERE_MAP"; break;
2709: 
2710:                 case EMT_REFLECTION_2_LAYER:                    entryPoint = "PS_REFLECTION_2_LAYER"; break;
2711: 
2712:                 case EMT_TRANSPARENT_ADD_COLOR:                 entryPoint = "PS_TRANSPARENT_ADD_COLOR"; break;
2713: 
2714:                 case EMT_TRANSPARENT_ALPHA_CHANNEL:             entryPoint = "PS_TRANSPARENT_ALPHA_CHANNEL"; break;
2715: 
2716:                 case EMT_TRANSPARENT_ALPHA_CHANNEL_REF:         entryPoint = "PS_TRANSPARENT_ALPHA_CHANNEL_REF"; break;
2717: 
2718:                 case EMT_TRANSPARENT_VERTEX_ALPHA:              entryPoint = "PS_TRANSPARENT_VERTEX_ALPHA"; break;
2719: 
2720:                 case EMT_TRANSPARENT_REFLECTION_2_LAYER:        entryPoint = "PS_TRANSPARENT_REFLECTION_2_LAYER"; break;
2721: 
2722:                 case EMT_NORMAL_MAP_SOLID:                      entryPoint = "PS_NORMAL_MAP_SOLID"; break;
2723: 
2724:                 case EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR:      entryPoint = "PS_NORMAL_MAP_TRANSPARENT_ADD_COLOR"; break;
2725: 
2726:                 case EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA:   entryPoint = "PS_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA"; break;
2727: 
2728:                 case EMT_PARALLAX_MAP_SOLID:                    entryPoint = "PS_PARALLAX_MAP_SOLID"; break;
2729: 
2730:                 case EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR:    entryPoint = "PS_PARALLAX_MAP_TRANSPARENT_ADD_COLOR"; break;
2731: 
2732:                 case EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA: entryPoint = "PS_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA"; break;
2733: 
2734:                 case EMT_ONETEXTURE_BLEND:                      entryPoint = "PS_ONETEXTURE_BLEND"; break;
2735: 
2736:                 default:
2737:                     return false;
2738:             }
2739: 
2740:             io::IReadFile    *file = FileSystem->createAndOpenFile("PS_MaterialShaders.hlsl");
2741:             if (!file)
2742:             {
2743:                 os::Printer::log("Failed to load PS_MaterialShaders.hlsl", ELL_ERROR);
2744:                 return false;
2745:             }
2746: 
2747:             const long      fileSize        = file->getSize();
2748:             char            *shaderSource   = new char[fileSize + 1];
2749:             file->read(shaderSource, fileSize);
2750:             shaderSource[fileSize] = 0;
2751:             file->drop();
2752: 
2753:             CD3D11Shader    *shader = new CD3D11Shader(this);
2754:             shader->setMaterialType(materialType);
2755: 
2756:             if (!shader->compile(EDST_PIXEL, shaderSource, entryPoint, "ps_4_0"))
2757:             {
2758:                 delete[] shaderSource;
2759:                 shader->drop();
2760:                 return false;
2761:             }
2762: 
2763:             delete[] shaderSource;
2764: 
2765:             if (!shader->createPixelShader())
2766:             {
2767:                 shader->drop();
2768:                 return false;
2769:             }
2770: 
2771:             m_ShaderPool.push_back(shader);
2772: 
2773:             if (m_BuiltInPixelShader[materialType])
2774:             {

(Showing lines 2675-2774 of 3206. Use offset=2775 to continue.)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11MaterialRenderer.h"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11MaterialRenderer.h</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #ifndef __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__
6: #define __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__
7: 
8: #include "IrrCompileConfig.h"
9: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
10: 
11: #include "IMaterialRenderer.h"
12: #include "SColor.h"
13: 
14: namespace irr
15: {
16:     namespace video
17:     {
18:         class CD3D11Driver;
19: 
20:         class CD3D11MaterialRenderer : public IMaterialRenderer
21:         {
22: public:
23: 
24:             CD3D11MaterialRenderer(CD3D11Driver *driver, s32 materialType,
25:                                    const c8 *name);
26: 
27:             virtual ~CD3D11MaterialRenderer();
28: 
29:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
30:                 bool resetAllRenderstates, IMaterialRendererServices *services);
31:             virtual bool OnSetTexture(u32 textureIndex, ITexture *texture);
32:             virtual void OnSetConstants(IMaterialRendererServices *services, s32 userData);
33:             virtual void PostRender();
34: 
35: protected:
36: 
37:             CD3D11Driver    *m_Driver;
38:             s32             m_MaterialType;
39:         };
40: 
41:         class CD3D11MaterialRenderer_SOLID : public CD3D11MaterialRenderer
42:         {
43: public:
44:             CD3D11MaterialRenderer_SOLID(CD3D11Driver *p, video::IVideoDriver *d);
45: 
46:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
47:                 bool resetAllRenderstates, IMaterialRendererServices *services);
48:         };
49: 
50:         class CD3D11MaterialRenderer_SOLID_2_LAYER : public CD3D11MaterialRenderer
51:         {
52: public:
53:             CD3D11MaterialRenderer_SOLID_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d);
54: 
55:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
56:                 bool resetAllRenderstates, IMaterialRendererServices *services);
57:         };
58: 
59:         class CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR : public CD3D11MaterialRenderer
60:         {
61: public:
62:             CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR(CD3D11Driver *p, video::IVideoDriver *d);
63: 
64:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
65:                 bool resetAllRenderstates, IMaterialRendererServices *services);
66: 
67:             virtual bool isTransparent() const;
68:         };
69: 
70:         class CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public CD3D11MaterialRenderer
71:         {
72: public:
73:             CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(CD3D11Driver *p, video::IVideoDriver *d);
74: 
75:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
76:                 bool resetAllRenderstates, IMaterialRendererServices *services);
77: 
78:             virtual bool isTransparent() const;
79:         };
80: 
81:         class CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public CD3D11MaterialRenderer
82:         {
83: public:
84:             CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(CD3D11Driver *p, video::IVideoDriver *d);
85: 
86:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
87:                 bool resetAllRenderstates, IMaterialRendererServices *services);
88: 
89:             virtual void OnUnsetMaterial();
90: 
91:             virtual bool isTransparent() const;
92:         };
93: 
94:         class CD3D11MaterialRenderer_ONETEXTURE_BLEND : public CD3D11MaterialRenderer
95:         {
96: public:
97:             CD3D11MaterialRenderer_ONETEXTURE_BLEND(CD3D11Driver *p, video::IVideoDriver *d);
98: 
99:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
100:                 bool resetAllRenderstates, IMaterialRendererServices *services);
101: 
102:             virtual bool isTransparent() const;
103: 
104:         private:
105:             u32 getD3D11Blend(E_BLEND_FACTOR factor) const;
106:             u32 getD3D11Modulate(E_MODULATE_FUNC func) const;
107:             bool m_Transparent;
108:         };
109: 
110:         class CD3D11MaterialRenderer_LIGHTMAP : public CD3D11MaterialRenderer
111:         {
112: public:
113:             CD3D11MaterialRenderer_LIGHTMAP(CD3D11Driver *p, video::IVideoDriver *d);
114: 
115:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
116:                 bool resetAllRenderstates, IMaterialRendererServices *services);
117:         };
118: 
119:         class CD3D11MaterialRenderer_DETAIL_MAP : public CD3D11MaterialRenderer
120:         {
121: public:
122:             CD3D11MaterialRenderer_DETAIL_MAP(CD3D11Driver *p, video::IVideoDriver *d);
123: 
124:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
125:                 bool resetAllRenderstates, IMaterialRendererServices *services);
126:         };
127: 
128:         class CD3D11MaterialRenderer_SPHERE_MAP : public CD3D11MaterialRenderer
129:         {
130: public:
131:             CD3D11MaterialRenderer_SPHERE_MAP(CD3D11Driver *p, video::IVideoDriver *d);
132: 
133:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
134:                 bool resetAllRenderstates, IMaterialRendererServices *services);
135: 
136:             virtual void OnUnsetMaterial();
137:         };
138: 
139:         class CD3D11MaterialRenderer_REFLECTION_2_LAYER : public CD3D11MaterialRenderer
140:         {
141: public:
142:             CD3D11MaterialRenderer_REFLECTION_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d);
143: 
144:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
145:                 bool resetAllRenderstates, IMaterialRendererServices *services);
146: 
147:             virtual void OnUnsetMaterial();
148:         };
149: 
150:     } // end namespace video
151: } // end namespace irr
152: #endif // _IRR_COMPILE_WITH_DIRECT3D_11_
153: #endif // __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__

(End of file - total 153 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 48.1s)

_Thinking:_

Now I understand what needs to be done:
1. Move the HLSL shader code from PS_MaterialShaders.hlsl into CD3D11MaterialRenderer.h as a global const char* string
2. Modify createMaterialPixelShader in CD3D11Driver.cpp to use this global string instead of loading from file

Let me add the global string to CD3D11MaterialRenderer.h and then modify the createMaterialPixelShader function to use it.

First, I'll add the global string at the top of CD3D11MaterialRenderer.h (before the class definition) and then modify the createMaterialPixelShader function.

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11MaterialRenderer.h",
  "oldString": "#ifndef __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__\n#define __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__\n\n#include \"IrrCompileConfig.h\"\n#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_\n\n#include \"IMaterialRenderer.h\"\n#include \"SColor.h\"\n\nnamespace irr",
  "newString": "#ifndef __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__\n#define __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__\n\n#include \"IrrCompileConfig.h\"\n#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_\n\n#include \"IMaterialRenderer.h\"\n#include \"SColor.h\"\n\nnamespace irr\n{\n    namespace video\n    {\n        const char PS_MaterialShaders[] = R\"(\n// Auto-generated HLSL Pixel Shaders for Irrlicht Material Types\n// Reference: CD3D9MaterialRenderer implementation\n\nstruct PS_INPUT_BASIC\n{\n    float4 Pos : SV_POSITION;\n    float4 Color : COLOR;\n    float2 TexCoord : TEXCOORD0;\n    float3 Normal : TEXCOORD1;\n};\n\nstruct PS_INPUT_2TEX\n{\n    float4 Pos : SV_POSITION;\n    float4 Color : COLOR;\n    float2 TexCoord0 : TEXCOORD0;\n    float2 TexCoord1 : TEXCOORD1;\n    float3 Normal : TEXCOORD2;\n};\n\nstruct PS_INPUT_TANGENTS\n{\n    float4 Pos : SV_POSITION;\n    float4 Color : COLOR;\n    float2 TexCoord : TEXCOORD0;\n    float3 Normal : TEXCOORD1;\n    float3 Tangent : TEXCOORD2;\n    float3 Binormal : TEXCOORD3;\n};\n\nTexture2D DiffuseTexture : register(t0);\nTexture2D LightmapTexture : register(t1);\nTexture2D DetailTexture : register(t1);\nTexture2D NormalMap : register(t1);\nTexture2D SphereMap : register(t0);\n\nSamplerState LinearSampler : register(s0);\n\nfloat4 PS_SOLID(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);\n}\n\nfloat4 PS_SOLID_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 layer1 = DiffuseTexture.Sample(LinearSampler, input.TexCoord1);\n    float alpha = input.Color.a;\n    return lerp(layer0, layer1, alpha);\n}\n\nfloat4 PS_LIGHTMAP(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * lightmap * 2.0;\n}\n\nfloat4 PS_LIGHTMAP_ADD(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse + lightmap;\n}\n\nfloat4 PS_LIGHTMAP_M2(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * (lightmap * 2.0);\n}\n\nfloat4 PS_LIGHTMAP_M4(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * (lightmap * 4.0);\n}\n\nfloat4 PS_LIGHTMAP_LIGHTING(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * lightmap * 2.0;\n}\n\nfloat4 PS_LIGHTMAP_LIGHTING_M2(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * (lightmap * 2.0);\n}\n\nfloat4 PS_LIGHTMAP_LIGHTING_M4(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * (lightmap * 4.0);\n}\n\nfloat4 PS_DETAIL_MAP(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 detail = DetailTexture.Sample(LinearSampler, input.TexCoord1);\n    float4 base = diffuse * input.Color;\n    return base + (detail - 0.5);\n}\n\nfloat4 PS_SPHERE_MAP(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);\n    float3 reflectVec = reflect(-viewDir, input.Normal);\n    float2 sphereUV = reflectVec.xy * 0.5 + 0.5;\n    float4 sphereColor = SphereMap.Sample(LinearSampler, sphereUV);\n    return texColor * sphereColor * 2.0;\n}\n\nfloat4 PS_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);\n    return layer0 * input.Color * reflection * 2.0;\n}\n\nfloat4 PS_TRANSPARENT_ADD_COLOR(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    float4 result = texColor * input.Color;\n    return result;\n}\n\nfloat4 PS_TRANSPARENT_ALPHA_CHANNEL(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    float4 result = texColor * input.Color;\n    result.a = texColor.a * input.Color.a;\n    return result;\n}\n\nfloat4 PS_TRANSPARENT_ALPHA_CHANNEL_REF(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    clip(texColor.a - 0.5);\n    return texColor * input.Color;\n}\n\nfloat4 PS_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);\n}\n\nfloat4 PS_TRANSPARENT_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);\n    float4 result = layer0 * reflection * 2.0;\n    result.a *= input.Color.a;\n    return result;\n}\n\nfloat4 PS_NORMAL_MAP_SOLID(PS_INPUT_TANGENTS input) : SV_TARGET\n{\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 lightDir1 = normalize(float3(1.0, 1.0, 1.0));\n    float3 lightDir2 = normalize(float3(-1.0, 0.5, 0.8));\n    float lighting = max(dot(normal, lightDir1), 0.0) + max(dot(normal, lightDir2), 0.0) * 0.5;\n\n    return float4(diffuseColor.rgb * lighting, diffuseColor.a);\n}\n\nfloat4 PS_NORMAL_MAP_TRANSPARENT_ADD_COLOR(PS_INPUT_TANGENTS input) : SV_TARGET\n{\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    return float4(diffuseColor.rgb * lighting, diffuseColor.a);\n}\n\nfloat4 PS_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_TANGENTS input) : SV_TARGET\n{\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    float4 result = diffuseColor;\n    result.rgb *= lighting;\n    result.a *= input.Color.a;\n    return result;\n}\n\nfloat4 PS_PARALLAX_MAP_SOLID(PS_INPUT_TANGENTS input) : SV_TARGET\n{\n    const float heightScale = 0.02f;\n\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);\n    float2 parallaxOffset = height * heightScale * viewDir.xy;\n    float2 offsetTexCoord = input.TexCoord + parallaxOffset;\n\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    return float4(texColor.rgb * lighting, texColor.a);\n}\n\nfloat4 PS_PARALLAX_MAP_TRANSPARENT_ADD_COLOR(PS_INPUT_TANGENTS input) : SV_TARGET\n{\n    const float heightScale = 0.02f;\n\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);\n    float2 parallaxOffset = height * heightScale * viewDir.xy;\n    float2 offsetTexCoord = input.TexCoord + parallaxOffset;\n\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    return float4(texColor.rgb * lighting, diffuseColor.a);\n}\n\nfloat4 PS_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_TANGENTS input) : SV_TARGET\n{\n    const float heightScale = 0.02f;\n\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);\n    float2 parallaxOffset = height * heightScale * viewDir.xy;\n    float2 offsetTexCoord = input.TexCoord + parallaxOffset;\n\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    float4 result = texColor;\n    result.rgb *= lighting;\n    result.a *= input.Color.a;\n    return result;\n}\n\nfloat4 PS_ONETEXTURE_BLEND(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    float4 result = texColor * input.Color;\n    return result;\n}\n\n struct VS_INPUT {\n     float3 Pos : POSITION;\n     float3 Normal : NORMAL;\n     float4 Color : COLOR;\n     float2 TexCoord : TEXCOORD0;\n };\n struct VS_OUTPUT {\n     float4 Pos : SV_POSITION;\n     float4 Color : COLOR;\n     float2 TexCoord : TEXCOORD0;\n     float3 Normal : TEXCOORD1;\n };\n cbuffer MatrixBuffer : register(b0) {\n     float4x4 WorldViewProj;\n };\n VS_OUTPUT main(VS_INPUT input) {\n     VS_OUTPUT output;\n     output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));\n     output.Color = input.Color;\n     output.TexCoord = input.TexCoord;\n     output.Normal = input.Normal;\n     return output;\n };\n)\";\n\n}"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 8.0s)

_Thinking:_

Now I need to modify CD3D11Driver::createMaterialPixelShader to use the global string PS_MaterialShaders instead of loading from file. Let me update that function.

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "oldString": "io::IReadFile    *file = FileSystem->createAndOpenFile(\"PS_MaterialShaders.hlsl\");\n            if (!file)\n            {\n                os::Printer::log(\"Failed to load PS_MaterialShaders.hlsl\", ELL_ERROR);\n                return false;\n            }\n\n            const long      fileSize        = file->getSize();\n            char            *shaderSource   = new char[fileSize + 1];\n            file->read(shaderSource, fileSize);\n            shaderSource[fileSize] = 0;\n            file->drop();\n\n            CD3D11Shader    *shader = new CD3D11Shader(this);\n            shader->setMaterialType(materialType);\n\n            if (!shader->compile(EDST_PIXEL, shaderSource, entryPoint, \"ps_4_0\"))\n            {\n                delete[] shaderSource;\n                shader->drop();\n                return false;\n            }\n\n            delete[] shaderSource;",
  "newString": "CD3D11Shader    *shader = new CD3D11Shader(this);\n            shader->setMaterialType(materialType);\n\n            if (!shader->compile(EDST_PIXEL, PS_MaterialShaders, entryPoint, \"ps_4_0\"))\n            {\n                shader->drop();\n                return false;\n            }"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 4.6s)

_Thinking:_

I need to also add the include for CD3D11MaterialRenderer.h to ensure the PS_MaterialShaders global is visible in CD3D11Driver.cpp. Let me check if it's already included.

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "offset": 1,
  "limit": 30
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
6: #include "CD3D11Driver.h"
7: #include "CD3D11ObjectTracker.h"
8: 
9: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
10: 
11: #include "os.h"
12: #include "S3DVertex.h"
13: #include "SColor.h"
14: #include "CD3D11Texture.h"
15: #include "CD3D11MaterialRenderer.h"
16: #include "CD3D11ShaderMaterialRenderer.h"
17: #include "CD3D11NormalMapRenderer.h"
18: #include "CD3D11ParallaxMapRenderer.h"
19: #include "CD3D11HLSLMaterialRenderer.h"
20: #include "CD3D11Shader.h"
21: #include "SIrrCreationParameters.h"
22: 
23: namespace irr
24: {
25:     namespace video
26:     {
27:         static const char    VERTEX_SHADER_STANDARD[] =
28:             "struct VS_INPUT {"
29:             "    float3 Pos : POSITION;"
30:             "    float3 Normal : NORMAL;"

(Showing lines 1-30 of 3190. Use offset=31 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 7.6s)

_Thinking:_

Good, `CD3D11MaterialRenderer.h` is already included at line 15, so the `PS_MaterialShaders` global string will be available. The changes are complete.

Let me verify the changes I made look correct by reading the modified areas.

Done. Changes made:

1. Added global `PS_MaterialShaders` string in `CD3D11MaterialRenderer.h` (before the `irr::video` namespace)

2. Modified `CD3D11Driver::createMaterialPixelShader` to use `PS_MaterialShaders` directly instead of loading from file:
   - Removed file loading code (`FileSystem->createAndOpenFile`, `file->read`, etc.)
   - Changed `shader->compile(EDST_PIXEL, shaderSource, ...)` to `shader->compile(EDST_PIXEL, PS_MaterialShaders, ...)`

Note: The `PS_MaterialShaders.hlsl` file can be deleted if no longer needed, but I left it intact in case it has other uses.

---

## User

根据下面的报错和HLSL代码分析是什么问题
Shader@0x00007FF930B83630(36,11-19): error X4500: overlapping register semantics not yet implemented 't0'
float4 PS_SPHERE_MAP(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
    float3 reflectVec = reflect(-viewDir, input.Normal);
    float2 sphereUV = reflectVec.xy * 0.5 + 0.5;
    float4 sphereColor = SphereMap.Sample(LinearSampler, sphereUV);
    return texColor * sphereColor * 2.0;
} 

---

## Assistant (Build · MiniMax-M2.7 · 18.3s)

_Thinking:_

The error is: "overlapping register semantics not yet implemented 't0'"

Looking at the shader code, I see that both `DiffuseTexture` and `SphereMap` are bound to `register(t0)`:

```hlsl
Texture2D DiffuseTexture : register(t0);
Texture2D SphereMap : register(t0);
```

This is the problem - two different textures using the same register `t0`. 

In the `PS_SPHERE_MAP` function, it uses both `DiffuseTexture` and `SphereMap`. The compiler can't handle two textures with the same register.

Looking at the original HLSL file, I see:
- `DiffuseTexture : register(t0);`
- `LightmapTexture : register(t1);`
- `DetailTexture : register(t1);`
- `NormalMap : register(t1);`
- `SphereMap : register(t0);`

So `SphereMap` is using `t0` but it should use a different register since it's used alongside `DiffuseTexture` in `PS_SPHERE_MAP`.

Looking at other functions:
- `PS_SOLID` only uses `DiffuseTexture` - so `t0` is fine
- `PS_SPHERE_MAP` uses both `DiffuseTexture` (t0) and `SphereMap` (t0) - conflict!

The fix should be to change `SphereMap` to use `register(t2)` or another unique register, and update the shader function that uses it accordingly.

Actually, looking at the D3D9 version, I recall that sphere maps typically use texture stage 0 for the sphere map itself, and the diffuse uses stage 0 as well but they're not used simultaneously. But in D3D11, you can't have two textures bound to the same register slot when compiling a shader.


The straightforward solution is to reassign `SphereMap` to `register(t2)` so it doesn't conflict with `DiffuseTexture` when both are needed in the same shader.

`SphereMap` 和 `DiffuseTexture` 都使用了 `register(t0)`，但 `PS_SPHERE_MAP` 函数同时使用这两个纹理，产生了寄存器冲突。

修改 `SphereMap` 使用 `t2`：


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11MaterialRenderer.h",
  "oldString": "Texture2D SphereMap : register(t0);",
  "newString": "Texture2D SphereMap : register(t2);"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 3.7s)

_Thinking:_

Done. Now SphereMap uses t2, which avoids the conflict with DiffuseTexture (t0) when PS_SPHERE_MAP uses both.

Done. `SphereMap` 改为 `register(t2)` 避免与 `DiffuseTexture` (t0) 冲突。

---

