# CD3D11Shader.cpp specular防负值修正

**Session ID:** ses_1d0dd87b8ffea0lOcAJ1oGJQel
**Created:** 2026/5/16 12:54:08
**Updated:** 2026/5/16 12:54:37

---

## User

把CD3D11Shader.cpp里面所有
            "    float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
都改为
            "    float3 specular = RdotV > 0.0 ? matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess) : float3(0.0, 0.0, 0.0);" 

---

## Assistant (Build · MiniMax-M2.7 · 7.7s)

_Thinking:_

The user wants to replace a specific line in CD3D11Shader.cpp. They want to change:
- from: `"    float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"`
- to: `"    float3 specular = RdotV > 0.0 ? matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess) : float3(0.0, 0.0, 0.0);"`

I need to first read the file to understand its structure, then use the edit tool to make the change. Since there might be multiple occurrences, I should use replaceAll if there are multiple instances.

Let me first read the file and then make the edit.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Shader.cpp"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Shader.cpp</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
6: #include "CD3D11Shader.h"
7: #include "CD3D11Driver.h"
8: #include "CD3D11ObjectTracker.h"
9: #include "os.h"
10: 
11: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
12: 
13: namespace irr
14: {
15:     namespace video
16:     {
17:         const char    VERTEX_SHADER_STANDARD[] =
18:             "struct VS_INPUT {"
19:             "    float3 Pos : POSITION;"
20:             "    float3 Normal : NORMAL;"
21:             "    float4 Color : COLOR;"
22:             "    float2 TexCoord : TEXCOORD0;"
23:             "};"
24:             "struct VS_OUTPUT {"
25:             "    float4 Pos : SV_POSITION;"
26:             "    float4 Color : COLOR;"
27:             "    float2 TexCoord : TEXCOORD0;"
28:             "    float2 TexCoord2 : TEXCOORD1;"
29:             "    float3 Normal : NORMAL;"
30:             "    float3 WorldPos : WORLDPOS;"
31:             "};"
32:             "cbuffer MatrixBuffer : register(b0) {"
33:             "    float4x4 WorldViewProj;"
34:             "    float4x4 World;"
35:             "};"
36:             "VS_OUTPUT main(VS_INPUT input) {"
37:             "    VS_OUTPUT output;"
38:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
39:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
40:             "    output.WorldPos = worldPos.xyz;"
41:             "    output.Color = input.Color;"
42:             "    output.TexCoord = input.TexCoord;"
43:             "    output.TexCoord2 = input.TexCoord;"
44:             "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
45:             "    return output;"
46:             "}";
47: 
48: const char    VERTEX_SHADER_STANDARD_DIRECTIONAL[] =
49:             "struct VS_INPUT {"
50:             "    float3 Pos : POSITION;"
51:             "    float3 Normal : NORMAL;"
52:             "    float4 Color : COLOR;"
53:             "    float2 TexCoord : TEXCOORD0;"
54:             "};"
55:             "struct VS_OUTPUT {"
56:             "    float4 Pos : SV_POSITION;"
57:             "    float4 Color : COLOR;"
58:             "    float2 TexCoord : TEXCOORD0;"
59:             "    float2 TexCoord2 : TEXCOORD1;"
60:             "    float3 Normal : NORMAL;"
61:             "    float3 WorldPos : TEXCOORD2;"
62:             "};"
63:             "cbuffer MatrixBuffer : register(b0) {"
64:             "    float4x4 WorldViewProj;"
65:             "    float4x4 World;"
66:             "};"
67:             "cbuffer LightBuffer : register(b1) {"
68:             "    float4 AmbientColor;"
69:             "    float4 DiffuseColor;"
70:             "    float4 SpecularColor;"
71:             "    float3 LightPos;"
72:             "    float LightRadius;"
73:             "    float3 LightDir;"
74:             "    float padding0;"
75:             "    float3 Attenuation;"
76:             "    float padding1;"
77:             "    float OuterCone;"
78:             "    float InnerCone;"
79:             "};"
80:             "cbuffer MaterialBuffer : register(b2) {"
81:             "    float4 MaterialDiffuseColor;"
82:             "    float4 MaterialAmbientColor;"
83:             "    float4 MaterialSpecularColor;"
84:             "    float4 MaterialEmissiveColor;"
85:             "    float Shininess;"
86:             "    uint ColorMaterialMode;"
87:             "    float2 Padding;"
88:             "};"
89:             "cbuffer CameraBuffer : register(b3) {"
90:             "    float3 cameraPos;"
91:             "};"
92:             "VS_OUTPUT main(VS_INPUT input) {"
93:             "    VS_OUTPUT output;"
94:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
95:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
96:             "    output.WorldPos = worldPos.xyz;"
97:             "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
98:             "    output.TexCoord = input.TexCoord;"
99:             "    output.TexCoord2 = input.TexCoord;"
100:             "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
101:             "    float3 lightDir = normalize(-LightDir.xyz);"
102:             "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
103:             "    float NdotL = max(dot(normal, lightDir), 0.0);"
104:             "    float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;"
105:             "    float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;"
106:             "    float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;"
107:             "    float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;"
108:             "    float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;"
109:             "    float3 R = reflect(-lightDir, normal);"
110:             "    float RdotV = max(dot(R, viewDir), 0.0);"
111:             "    float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
112:             "    float3 ambient = matAmbient.rgb * AmbientColor.rgb;"
113:             "    float3 emissive = matEmissive.rgb;"
114:             "    output.Color = float4(ambient + diffuse + specular + emissive, 1.0);"
115:             "    return output;"
116:             "}";
117: 
118:         const char    VERTEX_SHADER_STANDARD_POINT[] =
119:             "struct VS_INPUT {"
120:             "    float3 Pos : POSITION;"
121:             "    float3 Normal : NORMAL;"
122:             "    float4 Color : COLOR;"
123:             "    float2 TexCoord : TEXCOORD;"
124:             "};"
125:             "struct VS_OUTPUT {"
126:             "    float4 Pos : SV_POSITION;"
127:             "    float4 Color : COLOR;"
128:             "    float2 TexCoord : TEXCOORD0;"
129:             "    float2 TexCoord2 : TEXCOORD1;"
130:             "    float3 Normal : NORMAL;"
131:             "    float3 WorldPos : WORLDPOS;"
132:             "};"
133:             "cbuffer MatrixBuffer : register(b0) {"
134:             "    float4x4 WorldViewProj;"
135:             "    float4x4 World;"
136:             "};"
137:             "cbuffer LightBuffer : register(b1) {"
138:             "    float4 AmbientColor;"
139:             "    float4 DiffuseColor;"
140:             "    float4 SpecularColor;"
141:             "    float3 LightPos;"
142:             "    float LightRadius;"
143:             "    float3 LightDir;"
144:             "    float padding0;"
145:             "    float3 Attenuation;"
146:             "    float padding1;"
147:             "    float OuterCone;"
148:             "    float InnerCone;"
149:             "};"
150:             "cbuffer MaterialBuffer : register(b2) {"
151:             "    float4 MaterialDiffuseColor;"
152:             "    float4 MaterialAmbientColor;"
153:             "    float4 MaterialSpecularColor;"
154:             "    float4 MaterialEmissiveColor;"
155:             "    float Shininess;"
156:             "    uint ColorMaterialMode;"
157:             "    float2 Padding;"
158:             "};"
159:             "cbuffer CameraBuffer : register(b3) {"
160:             "    float3 cameraPos;"
161:             "};"
162:             "VS_OUTPUT main(VS_INPUT input) {"
163:             "    VS_OUTPUT output;"
164:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
165:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
166:             "    float3 nor = normalize(input.Normal);"
167:             "    float3 normal = normalize(mul(nor, (float3x3)World));"
168:             "    output.WorldPos = worldPos.xyz;"
169:             "    output.Normal = normal;"
170:             "    output.TexCoord = input.TexCoord;"
171:             "    output.TexCoord2 = input.TexCoord;"
172:             "    float3 lightDistant = LightPos.xyz - worldPos.xyz;"
173:             "    float dist = length(lightDistant);"
174:             "    float3 lightDir = normalize(lightDistant);"
175:             "    float3 viewDir = cameraPos;"
176:             "    float NdotL = max(dot(normal, lightDir), 0.0);"
177:             "    float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;"
178:             "    float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;"
179:             "    float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;"
180:             "    float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;"
181:             "    float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;"
182:             "    float3 R = reflect(lightDir, normal);"
183:             "    float RdotV = max(dot(R, viewDir), 0.0);"
184:             "    float3 specular = RdotV > 0.0 ? matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess) : float3(0.0, 0.0, 0.0);"
185:             "    float3 ambient = matAmbient.rgb * AmbientColor.rgb;"
186:             "    float3 emissive = matEmissive.rgb;"
187:             "    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);"
188:             "    float cond = dist <= LightRadius ? 1.0 : 0.0;"
189:             "    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * cond, 1.0);"
190:             "    return output;"
191:             "}";
192: 
193:         const char    VERTEX_SHADER_STANDARD_SPOT[] =
194:             "struct VS_INPUT {"
195:             "    float3 Pos : POSITION;"
196:             "    float3 Normal : NORMAL;"
197:             "    float4 Color : COLOR;"
198:             "    float2 TexCoord : TEXCOORD0;"
199:             "};"
200:             "struct VS_OUTPUT {"
201:             "    float4 Pos : SV_POSITION;"
202:             "    float4 Color : COLOR;"
203:             "    float2 TexCoord : TEXCOORD0;"
204:             "    float2 TexCoord2 : TEXCOORD1;"
205:             "    float3 Normal : NORMAL;"
206:             "    float3 WorldPos : WORLDPOS;"
207:             "};"
208:             "cbuffer MatrixBuffer : register(b0) {"
209:             "    float4x4 WorldViewProj;"
210:             "    float4x4 World;"
211:             "};"
212:             "cbuffer LightBuffer : register(b1) {"
213:             "    float4 AmbientColor;"
214:             "    float4 DiffuseColor;"
215:             "    float4 SpecularColor;"
216:             "    float3 LightPos;"
217:             "    float LightRadius;"
218:             "    float3 LightDir;"
219:             "    float padding0;"
220:             "    float3 Attenuation;"
221:             "    float padding1;"
222:             "    float OuterCone;"
223:             "    float InnerCone;"
224:             "};"
225:             "cbuffer MaterialBuffer : register(b2) {"
226:             "    float4 MaterialDiffuseColor;"
227:             "    float4 MaterialAmbientColor;"
228:             "    float4 MaterialSpecularColor;"
229:             "    float4 MaterialEmissiveColor;"
230:             "    float Shininess;"
231:             "    uint ColorMaterialMode;"
232:             "    float2 Padding;"
233:             "};"
234:             "cbuffer CameraBuffer : register(b3) {"
235:             "    float3 cameraPos;"
236:             "};"
237:             "VS_OUTPUT main(VS_INPUT input) {"
238:             "    VS_OUTPUT output;"
239:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
240:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
241:             "    output.WorldPos = worldPos.xyz;"
242:             "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
243:             "    output.TexCoord = input.TexCoord;"
244:             "    output.TexCoord2 = input.TexCoord;"
245:             "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
246:             "    float3 lightDir = LightPos.xyz - worldPos.xyz;"
247:             "    float dist = length(lightDir);"
248:             "    lightDir = normalize(lightDir);"
249:             "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
250:             "    float NdotL = max(dot(normal, lightDir), 0.0);"
251:             "    float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;"
252:             "    float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;"
253:             "    float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;"
254:             "    float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;"
255:             "    float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;"
256:             "    float3 R = reflect(-lightDir, normal);"
257:             "    float RdotV = max(dot(R, viewDir), 0.0);"
258:             "    float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
259:             "    float3 ambient = MaterialAmbientColor.rgb * AmbientColor.rgb;"
260:             "    float3 emissive = MaterialEmissiveColor.rgb;"
261:             "    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);"
262:             "    float cosAngle = dot(-lightDir, normalize(LightDir.xyz));"
263:             "    float cosOuter = cos(radians(OuterCone * 0.5));"
264:             "    float cosInner = cos(radians(InnerCone * 0.5));"
265:             "    float spotFactor = smoothstep(cosOuter, cosInner, cosAngle);"
266:             "    float cond = dist <= LightRadius ? 1.0 : 0.0;"
267:             "    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * spotFactor * cond, 1.0);"
268:             "    return output;"
269:             "}";
270: 
271:         const char    VERTEX_SHADER_2TCOORDS_DIRECTIONAL[] =
272:             "struct VS_INPUT {"
273:             "    float3 Pos : POSITION;"
274:             "    float3 Normal : NORMAL;"
275:             "    float4 Color : COLOR;"
276:             "    float2 TexCoord : TEXCOORD0;"
277:             "    float2 TexCoord2 : TEXCOORD1;"
278:             "};"
279:             "struct VS_OUTPUT {"
280:             "    float4 Pos : SV_POSITION;"
281:             "    float4 Color : COLOR;"
282:             "    float2 TexCoord : TEXCOORD0;"
283:             "    float2 TexCoord2 : TEXCOORD1;"
284:             "    float3 Normal : NORMAL;"
285:             "    float3 WorldPos : WORLDPOS;"
286:             "};"
287:             "cbuffer MatrixBuffer : register(b0) {"
288:             "    float4x4 WorldViewProj;"
289:             "    float4x4 World;"
290:             "};"
291:             "cbuffer LightBuffer : register(b1) {"
292:             "    float4 AmbientColor;"
293:             "    float4 DiffuseColor;"
294:             "    float4 SpecularColor;"
295:             "    float3 LightPos;"
296:             "    float LightRadius;"
297:             "    float3 LightDir;"
298:             "    float padding0;"
299:             "    float3 Attenuation;"
300:             "    float padding1;"
301:             "    float OuterCone;"
302:             "    float InnerCone;"
303:             "};"
304:             "cbuffer MaterialBuffer : register(b2) {"
305:             "    float4 MaterialDiffuseColor;"
306:             "    float4 MaterialAmbientColor;"
307:             "    float4 MaterialSpecularColor;"
308:             "    float4 MaterialEmissiveColor;"
309:             "    float Shininess;"
310:             "    uint ColorMaterialMode;"
311:             "    float2 Padding;"
312:             "};"
313:             "cbuffer CameraBuffer : register(b3) {"
314:             "    float3 cameraPos;"
315:             "};"
316:             "VS_OUTPUT main(VS_INPUT input) {"
317:             "    VS_OUTPUT output;"
318:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
319:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
320:             "    output.WorldPos = worldPos.xyz;"
321:             "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
322:             "    output.TexCoord = input.TexCoord;"
323:             "    output.TexCoord2 = input.TexCoord2;"
324:             "    float3 lightDir = -LightDir.xyz;"
325:             "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
326:             "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
327:             "    float NdotL = max(dot(normal, normalize(lightDir)), 0.0);"
328:             "    float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;"
329:             "    float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;"
330:             "    float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;"
331:             "    float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;"
332:             "    float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;"
333:             "    float3 R = reflect(-normalize(lightDir), normal);"
334:             "    float RdotV = max(dot(R, viewDir), 0.0);"
335:             "    float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
336:             "    float3 ambient = matAmbient.rgb * AmbientColor.rgb;"
337:             "    float3 emissive = matEmissive.rgb;"
338:             "    output.Color = float4(ambient + diffuse + specular + emissive, 1.0);"
339:             "    return output;"
340:             "}";
341: 
342:         const char    VERTEX_SHADER_2TCOORDS_POINT[] =
343:             "struct VS_INPUT {"
344:             "    float3 Pos : POSITION;"
345:             "    float3 Normal : NORMAL;"
346:             "    float4 Color : COLOR;"
347:             "    float2 TexCoord : TEXCOORD0;"
348:             "    float2 TexCoord2 : TEXCOORD1;"
349:             "};"
350:             "struct VS_OUTPUT {"
351:             "    float4 Pos : SV_POSITION;"
352:             "    float4 Color : COLOR;"
353:             "    float2 TexCoord : TEXCOORD0;"
354:             "    float2 TexCoord2 : TEXCOORD1;"
355:             "    float3 Normal : NORMAL;"
356:             "    float3 WorldPos : WORLDPOS;"
357:             "};"
358:             "cbuffer MatrixBuffer : register(b0) {"
359:             "    float4x4 WorldViewProj;"
360:             "    float4x4 World;"
361:             "};"
362:             "cbuffer LightBuffer : register(b1) {"
363:             "    float4 AmbientColor;"
364:             "    float4 DiffuseColor;"
365:             "    float4 SpecularColor;"
366:             "    float3 LightPos;"
367:             "    float LightRadius;"
368:             "    float3 LightDir;"
369:             "    float padding0;"
370:             "    float3 Attenuation;"
371:             "    float padding1;"
372:             "    float OuterCone;"
373:             "    float InnerCone;"
374:             "};"
375:             "cbuffer MaterialBuffer : register(b2) {"
376:             "    float4 MaterialDiffuseColor;"
377:             "    float4 MaterialAmbientColor;"
378:             "    float4 MaterialSpecularColor;"
379:             "    float4 MaterialEmissiveColor;"
380:             "    float Shininess;"
381:             "    uint ColorMaterialMode;"
382:             "    float2 Padding;"
383:             "};"
384:             "cbuffer CameraBuffer : register(b3) {"
385:             "    float3 cameraPos;"
386:             "};"
387:             "VS_OUTPUT main(VS_INPUT input) {"
388:             "    VS_OUTPUT output;"
389:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
390:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
391:             "    output.WorldPos = worldPos.xyz;"
392:             "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
393:             "    output.TexCoord = input.TexCoord;"
394:             "    output.TexCoord2 = input.TexCoord2;"
395:             "    float3 lightDir = LightPos.xyz - worldPos.xyz;"
396:             "    float dist = length(lightDir);"
397:             "    lightDir = normalize(lightDir);"
398:             "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
399:             "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
400:             "    float NdotL = max(dot(normal, lightDir), 0.0);"
401:             "    float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;"
402:             "    float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;"
403:             "    float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;"
404:             "    float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;"
405:             "    float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;"
406:             "    float3 R = reflect(-lightDir, normal);"
407:             "    float RdotV = max(dot(R, viewDir), 0.0);"
408:             "    float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
409:             "    float3 ambient = matAmbient.rgb * AmbientColor.rgb;"
410:             "    float3 emissive = matEmissive.rgb;"
411:             "    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);"
412:             "    float cond = dist <= LightRadius ? 1.0 : 0.0;"
413:             "    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * cond, 1.0);"
414:             "    return output;"
415:             "}";
416: 
417: const char    VERTEX_SHADER_2TCOORDS_SPOT[] =
418:             "struct VS_INPUT {"
419:             "    float3 Pos : POSITION;"
420:             "    float3 Normal : NORMAL;"
421:             "    float4 Color : COLOR;"
422:             "    float2 TexCoord : TEXCOORD0;"
423:             "    float2 TexCoord2 : TEXCOORD1;"
424:             "};"
425:             "struct VS_OUTPUT {"
426:             "    float4 Pos : SV_POSITION;"
427:             "    float4 Color : COLOR;"
428:             "    float2 TexCoord : TEXCOORD0;"
429:             "    float2 TexCoord2 : TEXCOORD1;"
430:             "    float3 Normal : NORMAL;"
431:             "    float3 WorldPos : WORLDPOS;"
432:             "};"
433:             "cbuffer MatrixBuffer : register(b0) {"
434:             "    float4x4 WorldViewProj;"
435:             "    float4x4 World;"
436:             "};"
437:             "cbuffer LightBuffer : register(b1) {"
438:             "    float4 AmbientColor;"
439:             "    float4 DiffuseColor;"
440:             "    float4 SpecularColor;"
441:             "    float3 LightPos;"
442:             "    float LightRadius;"
443:             "    float3 LightDir;"
444:             "    float padding0;"
445:             "    float3 Attenuation;"
446:             "    float padding1;"
447:             "    float OuterCone;"
448:             "    float InnerCone;"
449:             "};"
450:             "cbuffer MaterialBuffer : register(b2) {"
451:             "    float4 MaterialDiffuseColor;"
452:             "    float4 MaterialAmbientColor;"
453:             "    float4 MaterialSpecularColor;"
454:             "    float4 MaterialEmissiveColor;"
455:             "    float Shininess;"
456:             "    uint ColorMaterialMode;"
457:             "    float2 Padding;"
458:             "};"
459:             "cbuffer CameraBuffer : register(b3) {"
460:             "    float3 cameraPos;"
461:             "};"
462:             "VS_OUTPUT main(VS_INPUT input) {"
463:             "    VS_OUTPUT output;"
464:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
465:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
466:             "    output.WorldPos = worldPos.xyz;"
467:             "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
468:             "    output.TexCoord = input.TexCoord;"
469:             "    output.TexCoord2 = input.TexCoord2;"
470:             "    float3 lightDir = LightPos.xyz - worldPos.xyz;"
471:             "    float dist = length(lightDir);"
472:             "    lightDir = normalize(lightDir);"
473:             "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
474:             "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
475:             "    float NdotL = max(dot(normal, lightDir), 0.0);"
476:             "    float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;"
477:             "    float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;"
478:             "    float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;"
479:             "    float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;"
480:             "    float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;"
481:             "    float3 R = reflect(-lightDir, normal);"
482:             "    float RdotV = max(dot(R, viewDir), 0.0);"
483:             "    float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
484:             "    float3 ambient = matAmbient.rgb * AmbientColor.rgb;"
485:             "    float3 emissive = matEmissive.rgb;"
486:             "    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);"
487:             "    float cosAngle = dot(-lightDir, normalize(LightDir.xyz));"
488:             "    float cosOuter = cos(radians(OuterCone * 0.5));"
489:             "    float cosInner = cos(radians(InnerCone * 0.5));"
490:             "    float spotFactor = smoothstep(cosOuter, cosInner, cosAngle);"
491:             "    float cond = dist <= LightRadius ? 1.0 : 0.0;"
492:             "    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * spotFactor * cond, 1.0);"
493:             "    return output;"
494:             "}";
495: 
496:         const char    VERTEX_SHADER_2TCOORDS[] =
497:             "struct VS_INPUT {"
498:             "    float3 Pos : POSITION;"
499:             "    float3 Normal : NORMAL;"
500:             "    float4 Color : COLOR;"
501:             "    float2 TexCoord : TEXCOORD0;"
502:             "    float2 TexCoord2 : TEXCOORD1;"
503:             "};"
504:             "struct VS_OUTPUT {"
505:             "    float4 Pos : SV_POSITION;"
506:             "    float4 Color : COLOR;"
507:             "    float2 TexCoord : TEXCOORD0;"
508:             "    float2 TexCoord2 : TEXCOORD1;"
509:             "    float3 Normal : NORMAL;"
510:             "    float3 WorldPos : WORLDPOS;"
511:             "};"
512:             "cbuffer MatrixBuffer : register(b0) {"
513:             "    float4x4 WorldViewProj;"
514:             "    float4x4 World;"
515:             "};"
516:             "VS_OUTPUT main(VS_INPUT input) {"
517:             "    VS_OUTPUT output;"
518:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
519:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
520:             "    output.WorldPos = worldPos.xyz;"
521:             "    output.Color = input.Color;"
522:             "    output.TexCoord = input.TexCoord;"
523:             "    output.TexCoord2 = input.TexCoord2;"
524:             "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
525:             "    return output;"
526:             "}";
527: 
528:         const char    VERTEX_SHADER_RECTANGLE[] =
529:             "cbuffer MatrixBuffer : register(b0) {"
530:             "    float4x4 WorldViewProj;"
531:             "    float4x4 World;"
532:             "};"
533:             "struct VS_INPUT {"
534:             "    float3 Pos : POSITION;"
535:             "    float4 Color : COLOR;"
536:             "};"
537:             "struct VS_OUTPUT {"
538:             "    float4 Pos : SV_POSITION;"
539:             "    float4 Color : COLOR;"
540:             "    float3 WorldPos : WORLDPOS;"
541:             "};"
542:             "VS_OUTPUT main(VS_INPUT input) {"
543:             "    VS_OUTPUT output;"
544:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
545:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
546:             "    output.WorldPos = worldPos.xyz;"
547:             "    output.Color = input.Color;"
548:             "    return output;"
549:             "}";
550: 
551:         const char    PIXEL_SHADER_RECTANGLE[] =
552:             "struct PS_INPUT {"
553:             "    float4 Pos : SV_POSITION;"
554:             "    float4 Color : COLOR;"
555:             "    float3 WorldPos : WORLDPOS;"
556:             "};"
557:             "float4 main(PS_INPUT input) : SV_TARGET {"
558:             "    return input.Color;"
559:             "}";
560: 
561:         const char    VERTEX_SHADER_TANGENTS[] =
562:             "struct VS_INPUT {"
563:             "    float3 Pos : POSITION;"
564:             "    float3 Normal : NORMAL;"
565:             "    float4 Color : COLOR;"
566:             "    float2 TexCoord : TEXCOORD0;"
567:             "    float3 Tangent : TANGENT;"
568:             "    float3 Binormal : BINORMAL;"
569:             "};"
570:             "struct VS_OUTPUT {"
571:             "    float4 Pos : SV_POSITION;"
572:             "    float4 Color : COLOR;"
573:             "    float2 TexCoord : TEXCOORD0;"
574:             "    float3 Normal : NORMAL;"
575:             "    float3 Tangent : TEXCOORD2;"
576:             "    float3 Binormal : TEXCOORD3;"
577:             "    float3 WorldPos : WORLDPOS;"
578:             "};"
579:             "cbuffer MatrixBuffer : register(b0) {"
580:             "    float4x4 WorldViewProj;"
581:             "    float4x4 World;"
582:             "};"
583:             "VS_OUTPUT main(VS_INPUT input) {"
584:             "    VS_OUTPUT output;"
585:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
586:             "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
587:             "    output.WorldPos = worldPos.xyz;"
588:             "    output.Color = input.Color;"
589:             "    output.TexCoord = input.TexCoord;"
590:             "    output.Normal = input.Normal;"
591:             "    output.Tangent = input.Tangent;"
592:             "    output.Binormal = input.Binormal;"
593:             "    return output;"
594:             "}";
595: 
596:         const char    PIXEL_SHADER_STANDARD[] =
597:             "cbuffer MatrixBuffer : register(b0) {"
598:             "    float4x4 WorldViewProj;"
599:             "};"
600:             "struct PS_INPUT {"
601:             "    float4 Pos : SV_POSITION;"
602:             "    float4 Color : COLOR;"
603:             "    float2 TexCoord : TEXCOORD0;"
604:             "    float3 Normal : TEXCOORD1;"
605:             "    float3 WorldPos : WORLDPOS;"
606:             "};"
607:             "Texture2D DiffuseTexture : register(t0);"
608:             "SamplerState LinearSampler : register(s0);"
609:             "float4 main(PS_INPUT input) : SV_TARGET {"
610:             "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
611:             "    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);"
612:             "}";
613: 
614:         const char    PIXEL_SHADER_2TCOORDS[] =
615:             "cbuffer MatrixBuffer : register(b0) {"
616:             "    float4x4 WorldViewProj;"
617:             "};"
618:             "struct PS_INPUT {"
619:             "    float4 Pos : SV_POSITION;"
620:             "    float4 Color : COLOR;"
621:             "    float2 TexCoord : TEXCOORD0;"
622:             "    float2 TexCoord2 : TEXCOORD1;"
623:             "    float3 Normal : NORMAL;"
624:             "    float3 WorldPos : WORLDPOS;"
625:             "};"
626:             "Texture2D DiffuseTexture : register(t0);"
627:             "SamplerState LinearSampler : register(s0);"
628:             "float4 main(PS_INPUT input) : SV_TARGET {"
629:             "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
630:             "    return input.Color * texColor;"
631:             "}";
632: 
633:         const char    PIXEL_SHADER_TANGENTS[] =
634:             "cbuffer MatrixBuffer : register(b0) {"
635:             "    float4x4 WorldViewProj;"
636:             "};"
637:             "struct PS_INPUT {"
638:             "    float4 Pos : SV_POSITION;"
639:             "    float4 Color : COLOR;"
640:             "    float2 TexCoord : TEXCOORD0;"
641:             "    float3 Normal : NORMAL;"
642:             "    float3 Tangent : TEXCOORD2;"
643:             "    float3 Binormal : TEXCOORD3;"
644:             "    float3 WorldPos : WORLDPOS;"
645:             "};"
646:             "Texture2D DiffuseTexture : register(t0);"
647:             "SamplerState LinearSampler : register(s0);"
648:             "float4 main(PS_INPUT input) : SV_TARGET {"
649:             "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
650:             "    return input.Color * texColor;"
651:             "}";
652: 
653:         CD3D11Shader::CD3D11Shader(CD3D11Driver *driver)
654:             : m_Driver(driver), m_ShaderType(EDST_COUNT), m_VertexType(EVT_STANDARD),
655:             m_MaterialType(EMT_SOLID), m_Compiled(false),
656:             m_ShaderBlob(0), m_VertexShader(0), m_HullShader(0), m_DomainShader(0),
657:             m_GeometryShader(0), m_PixelShader(0), m_ComputeShader(0),
658:             m_InputLayout(0), m_InputLayoutDesc(0), m_InputLayoutElementCount(0)
659:         {
660: #ifdef _DEBUG
661:             setDebugName("CD3D11Shader");
662: #endif
663:         }
664: 
665:         CD3D11Shader::~CD3D11Shader()
666:         {
667:             if (m_InputLayout)
668:             {
669:                 IRR_D3D11_IL_RELEASE(m_InputLayout, "InputLayout");
670:                 m_InputLayout->Release();
671:             }
672: 
673:             if (m_InputLayoutDesc)
674:                 delete[] m_InputLayoutDesc;
675: 
676:             if (m_ShaderBlob)
677:                 m_ShaderBlob->Release();
678: 
679:             if (m_VertexShader)
680:             {
681:                 IRR_D3D11_VS_RELEASE(m_VertexShader, "VertexShader");
682:                 m_VertexShader->Release();
683:             }
684: 
685:             if (m_HullShader)
686:             {
687:                 IRR_D3D11_HS_RELEASE(m_HullShader, "HullShader");
688:                 m_HullShader->Release();
689:             }
690: 
691:             if (m_DomainShader)
692:             {
693:                 IRR_D3D11_DS_RELEASE(m_DomainShader, "DomainShader");
694:                 m_DomainShader->Release();
695:             }
696: 
697:             if (m_GeometryShader)
698:             {
699:                 IRR_D3D11_GS_RELEASE(m_GeometryShader, "GeometryShader");
700:                 m_GeometryShader->Release();
701:             }
702: 
703:             if (m_PixelShader)
704:             {
705:                 IRR_D3D11_PS_RELEASE(m_PixelShader, "PixelShader");
706:                 m_PixelShader->Release();
707:             }
708: 
709:             if (m_ComputeShader)
710:             {
711:                 IRR_D3D11_CS_RELEASE(m_ComputeShader, "ComputeShader");
712:                 m_ComputeShader->Release();
713:             }
714:         }
715: 
716:         void CD3D11Shader::drop()
717:         {
718:             IReferenceCounted::drop();
719:         }
720: 
721:         bool CD3D11Shader::compile(E_D3D11_SHADER_TYPE type, const c8 *hlslSource, const c8 *entryPoint, const c8 *profile, const c8 *hlslSourcePart2)
722:         {
723:             if (!hlslSource || !entryPoint || !profile)
724:                 return false;
725: 
726:             if (hlslSourcePart2)
727:             {
728:                 m_HLSLSource = hlslSource;
729:                 m_HLSLSource += hlslSourcePart2;
730:             }
731:             else
732:             {
733:                 m_HLSLSource = hlslSource;
734:             }
735: 
736:             m_EntryPoint    = entryPoint;
737:             m_Profile       = profile;
738:             m_ShaderType    = type;
739: 
740:             ID3DBlob    *errorBlob = 0;
741: 
742:             const c8* sourceToCompile = hlslSource;
743:             size_t sourceLen = strlen(hlslSource);
744:             if (hlslSourcePart2)
745:             {
746:                 sourceToCompile = m_HLSLSource.c_str();
747:                 sourceLen = m_HLSLSource.size();
748:             }
749: 
750:             HRESULT    hr = D3DCompile(sourceToCompile, sourceLen, 0, 0, 0, entryPoint,
751:                                        profile, D3DCOMPILE_SKIP_VALIDATION, 0, &m_ShaderBlob, &errorBlob);
752: 
753:             if (FAILED(hr))
754:             {
755:                 if (errorBlob)
756:                 {
757:                     os::Printer::log("Shader compilation failed:", ELL_ERROR);
758:                     os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);
759:                     errorBlob->Release();
760:                 }
761: 
762:                 return false;
763:             }
764: 
765:             m_Compiled = true;
766:             return true;
767:         }
768: 
769:         bool CD3D11Shader::createVertexShader()
770:         {
771:             if (!m_Compiled || m_ShaderType != EDST_VERTEX || !m_ShaderBlob)
772:                 return false;
773: 
774:             if (m_VertexShader)
775:             {
776:                 m_VertexShader->Release();
777:                 m_VertexShader = 0;
778:             }
779: 
780:             HRESULT    hr = m_Driver->m_pID3DDevice->CreateVertexShader(
781:                 m_ShaderBlob->GetBufferPointer(),
782:                 m_ShaderBlob->GetBufferSize(),
783:                 nullptr,
784:                 &m_VertexShader);
785: 
786:             IRR_D3D11_VS_CREATE(m_VertexShader, "VertexShader");
787:             return SUCCEEDED(hr);
788:         }
789: 
790:         bool CD3D11Shader::createHullShader()
791:         {
792:             if (!m_Compiled || m_ShaderType != EDST_HULL || !m_ShaderBlob)
793:                 return false;
794: 
795:             if (m_HullShader)
796:             {
797:                 m_HullShader->Release();
798:                 m_HullShader = 0;
799:             }
800: 
801:             HRESULT    hr = m_Driver->m_pID3DDevice->CreateHullShader(
802:                 m_ShaderBlob->GetBufferPointer(),
803:                 m_ShaderBlob->GetBufferSize(),
804:                 nullptr,
805:                 &m_HullShader);
806: 
807:             IRR_D3D11_HS_CREATE(m_HullShader, "HullShader");
808:             return SUCCEEDED(hr);
809:         }
810: 
811:         bool CD3D11Shader::createDomainShader()
812:         {
813:             if (!m_Compiled || m_ShaderType != EDST_DOMAIN || !m_ShaderBlob)
814:                 return false;
815: 
816:             if (m_DomainShader)
817:             {
818:                 m_DomainShader->Release();
819:                 m_DomainShader = 0;
820:             }
821: 
822:             HRESULT    hr = m_Driver->m_pID3DDevice->CreateDomainShader(
823:                 m_ShaderBlob->GetBufferPointer(),
824:                 m_ShaderBlob->GetBufferSize(),
825:                 nullptr,
826:                 &m_DomainShader);
827: 
828:             IRR_D3D11_DS_CREATE(m_DomainShader, "DomainShader");
829:             return SUCCEEDED(hr);
830:         }
831: 
832:         bool CD3D11Shader::createGeometryShader()
833:         {
834:             if (!m_Compiled || m_ShaderType != EDST_GEOMETRY || !m_ShaderBlob)
835:                 return false;
836: 
837:             if (m_GeometryShader)
838:             {
839:                 m_GeometryShader->Release();
840:                 m_GeometryShader = 0;
841:             }
842: 
843:             HRESULT    hr = m_Driver->m_pID3DDevice->CreateGeometryShader(
844:                 m_ShaderBlob->GetBufferPointer(),
845:                 m_ShaderBlob->GetBufferSize(),
846:                 nullptr,
847:                 &m_GeometryShader);
848: 
849:             IRR_D3D11_GS_CREATE(m_GeometryShader, "GeometryShader");
850:             return SUCCEEDED(hr);
851:         }
852: 
853:         bool CD3D11Shader::createPixelShader()
854:         {
855:             if (!m_Compiled || m_ShaderType != EDST_PIXEL || !m_ShaderBlob)
856:                 return false;
857: 
858:             if (m_PixelShader)
859:             {
860:                 m_PixelShader->Release();
861:                 m_PixelShader = 0;
862:             }
863: 
864:             HRESULT    hr = m_Driver->m_pID3DDevice->CreatePixelShader(
865:                 m_ShaderBlob->GetBufferPointer(),
866:                 m_ShaderBlob->GetBufferSize(),
867:                 nullptr,
868:                 &m_PixelShader);
869: 
870:             IRR_D3D11_PS_CREATE(m_PixelShader, "PixelShader");
871:             return SUCCEEDED(hr);
872:         }
873: 
874:         bool CD3D11Shader::createComputeShader()
875:         {
876:             if (!m_Compiled || m_ShaderType != EDST_COMPUTE || !m_ShaderBlob)
877:                 return false;
878: 
879:             if (m_ComputeShader)
880:             {
881:                 m_ComputeShader->Release();
882:                 m_ComputeShader = 0;
883:             }
884: 
885:             HRESULT    hr = m_Driver->m_pID3DDevice->CreateComputeShader(
886:                 m_ShaderBlob->GetBufferPointer(),
887:                 m_ShaderBlob->GetBufferSize(),
888:                 nullptr,
889:                 &m_ComputeShader);
890: 
891:             IRR_D3D11_CS_CREATE(m_ComputeShader, "ComputeShader");
892:             return SUCCEEDED(hr);
893:         }
894: 
895:         bool CD3D11Shader::createInputLayout(const D3D11_INPUT_ELEMENT_DESC *layout, u32 elementCount)
896:         {
897:             if (!m_Compiled || m_ShaderType != EDST_VERTEX || !m_ShaderBlob)
898:                 return false;
899: 
900:             if (m_InputLayout)
901:             {
902:                 m_InputLayout->Release();
903:                 m_InputLayout = 0;
904:             }
905: 
906:             if (m_InputLayoutDesc)
907:             {
908:                 delete[] m_InputLayoutDesc;
909:                 m_InputLayoutDesc = 0;
910:             }
911: 
912:             m_InputLayoutDesc = new D3D11_INPUT_ELEMENT_DESC[elementCount];
913:             memcpy(m_InputLayoutDesc, layout, sizeof(D3D11_INPUT_ELEMENT_DESC) * elementCount);
914:             m_InputLayoutElementCount = elementCount;
915: 
916:             HRESULT    hr = m_Driver->m_pID3DDevice->CreateInputLayout(
917:                 layout,
918:                 elementCount,
919:                 m_ShaderBlob->GetBufferPointer(),
920:                 m_ShaderBlob->GetBufferSize(),
921:                 &m_InputLayout);
922: 
923:             IRR_D3D11_IL_CREATE(m_InputLayout, "InputLayout");
924:             return SUCCEEDED(hr);
925:         }
926:     } // end namespace video
927: } // end namespace irr
928: #endif // _IRR_COMPILE_WITH_DIRECT3D_11_

(End of file - total 928 lines)
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

---

## Assistant (Build · MiniMax-M2.7 · 11.0s)

_Thinking:_

The user wants to replace all instances of:
```
    float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);
```

with:
```
    float3 specular = RdotV > 0.0 ? matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess) : float3(0.0, 0.0, 0.0);
```

I can see from the file that line 111, 258, 335, 408, 483 have the original pattern. But line 184 already has the new pattern.

Let me use replaceAll to change all instances of the old pattern.


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Shader.cpp",
  "newString": "float3 specular = RdotV > 0.0 ? matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess) : float3(0.0, 0.0, 0.0);",
  "oldString": "float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);",
  "replaceAll": true
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 9.4s)

_Thinking:_

The edit was successful. I replaced all instances of the original specular calculation with the conditional version that checks if RdotV > 0.0 before computing the specular term.

Let me verify by checking if there are any remaining instances of the original pattern.




已完成。文件中有5处需要修改的地方，现在都已改为带 RdotV > 0.0 条件判断的版本。

---

