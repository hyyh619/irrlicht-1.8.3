# New session - 2026-05-13T08:09:17.582Z

**Session ID:** ses_1df9df1f2ffeNsSceybLHejuSc
**Created:** 2026/5/13 16:09:17
**Updated:** 2026/5/13 16:13:01

---

## User

帮我分析一下cameraPosition和viewMatrix在Scene中是如何被计算得到的。
1. 请使用07.Collision，camera设置在main.cpp文件中

---

## Assistant (Build · MiniMax-M2.7 · 31.6s)

---

## User

帮我分析一下cameraPosition和viewMatrix在Scene中是如何被计算得到的。
1. 请使用07.Collision，camera设置在main.cpp文件中

---

## Assistant (Build · MiniMax-M2.7 · 11.3s)

_Thinking:_

用户想要分析cameraPosition和viewMatrix在Scene中是如何被计算的，使用07.Collision示例，camera设置在main.cpp文件中。

让我先读取07.Collision的main.cpp文件来了解camera是如何设置的。


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\examples\\07.Collision\\main.cpp"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\examples\07.Collision\main.cpp</path>
<type>file</type>
<content>
1: /** Example 007 Collision
2: 
3:    We will describe 2 methods: Automatic collision detection for moving through
4:    3d worlds with stair climbing and sliding, and manual scene node and triangle
5:    picking using a ray.  In this case, we will use a ray coming out from the
6:    camera, but you can use any ray.
7: 
8:    To start, we take the program from tutorial 2, which loads and displays a
9:    quake 3 level. We will use the level to walk in it and to pick triangles from.
10:    In addition we'll place 3 animated models into it for triangle picking. The
11:    following code starts up the engine and loads the level, as per tutorial 2.
12:  */
13: #include <irrlicht.h>
14: #include "driverChoice.h"
15: 
16: using namespace irr;
17: 
18: #ifdef _MSC_VER
19: #pragma comment(lib, "Irrlicht.lib")
20: #endif
21: 
22: #ifndef CONFORM_TEST_DX9
23: #define CONFORM_TEST_DX9 0
24: #endif
25: 
26: #ifndef CONFORM_TEST_DX11
27: #define CONFORM_TEST_DX11 0
28: #endif
29: 
30: enum
31: {
32:     // I use this ISceneNode ID to indicate a scene node that is
33:     // not pickable by getSceneNodeAndCollisionPointFromRay()
34:     ID_IsNotPickable = 0,
35: 
36:     // I use this flag in ISceneNode IDs to indicate that the
37:     // scene node can be picked by ray selection.
38:     IDFlag_IsPickable = 1 << 0,
39: 
40:     // I use this flag in ISceneNode IDs to indicate that the
41:     // scene node can be highlighted.  In this example, the
42:     // homonids can be highlighted, but the level mesh can't.
43:     IDFlag_IsHighlightable = 1 << 1
44: };
45: 
46: int main()
47: {
48:     // ask user for driver
49: 
50: #if WIN32 
51: #if CONFORM_TEST_DX9
52:     video::E_DRIVER_TYPE driverType = video::EDT_DIRECT3D9;
53: #elif CONFORM_TEST_DX11
54:     video::E_DRIVER_TYPE driverType = video::EDT_DIRECT3D11;
55: #else
56:     video::E_DRIVER_TYPE driverType = video::EDT_DIRECT3D11;
57: #endif
58: #else
59:     video::E_DRIVER_TYPE driverType = driverChoiceConsole();
60: #endif
61: 
62:     if (driverType==video::EDT_COUNT)
63:         return 1;
64: 
65:     // create device
66: 
67:     IrrlichtDevice *device =
68:         createDevice(driverType, core::dimension2d<u32>(640, 480), 16, false);
69: 
70:     if (device == 0)
71:         return 1;         // could not create selected driver.
72: 
73:     video::IVideoDriver  *driver = device->getVideoDriver();
74:     scene::ISceneManager *smgr   = device->getSceneManager();
75: 
76:     device->getFileSystem()->addFileArchive("../../media/map-20kdm2.pk3");
77: 
78:     scene::IAnimatedMesh  *q3levelmesh = smgr->getMesh("20kdm2.bsp");
79:     scene::IMeshSceneNode *q3node      = 0;
80: 
81:     // The Quake mesh is pickable, but doesn't get highlighted.
82:     if (q3levelmesh)
83:         q3node = smgr->addOctreeSceneNode(q3levelmesh->getMesh(0), 0, IDFlag_IsPickable);
84: 
85:     /*
86:        So far so good, we've loaded the quake 3 level like in tutorial 2. Now,
87:        here comes something different: We create a triangle selector. A
88:        triangle selector is a class which can fetch the triangles from scene
89:        nodes for doing different things with them, for example collision
90:        detection. There are different triangle selectors, and all can be
91:        created with the ISceneManager. In this example, we create an
92:        OctreeTriangleSelector, which optimizes the triangle output a little
93:        bit by reducing it like an octree. This is very useful for huge meshes
94:        like quake 3 levels. After we created the triangle selector, we attach
95:        it to the q3node. This is not necessary, but in this way, we do not
96:        need to care for the selector, for example dropping it after we do not
97:        need it anymore.
98:      */
99: 
100:     scene::ITriangleSelector *selector = 0;
101: 
102:     if (q3node)
103:     {
104:         q3node->setPosition(core::vector3df(-1350, -130, -1400));
105: 
106:         selector = smgr->createOctreeTriangleSelector(
107:             q3node->getMesh(), q3node, 128);
108:         q3node->setTriangleSelector(selector);
109:         // We're not done with this selector yet, so don't drop it.
110:     }
111: 
112: 
113:     /*
114:        We add a first person shooter camera to the scene so that we can see and
115:        move in the quake 3 level like in tutorial 2. But this, time, we add a
116:        special animator to the camera: A Collision Response animator. This
117:        animator modifies the scene node to which it is attached to in order to
118:        prevent it moving through walls, and to add gravity to it. The
119:        only thing we have to tell the animator is how the world looks like,
120:        how big the scene node is, how much gravity to apply and so on. After the
121:        collision response animator is attached to the camera, we do not have to do
122:        anything more for collision detection, anything is done automatically.
123:        The rest of the collision detection code below is for picking. And please
124:        note another cool feature: The collision response animator can be
125:        attached also to all other scene nodes, not only to cameras. And it can
126:        be mixed with other scene node animators. In this way, collision
127:        detection and response in the Irrlicht engine is really easy.
128: 
129:        Now we'll take a closer look on the parameters of
130:        createCollisionResponseAnimator(). The first parameter is the
131:        TriangleSelector, which specifies how the world, against collision
132:        detection is done looks like. The second parameter is the scene node,
133:        which is the object, which is affected by collision detection, in our
134:        case it is the camera. The third defines how big the object is, it is
135:        the radius of an ellipsoid. Try it out and change the radius to smaller
136:        values, the camera will be able to move closer to walls after this. The
137:        next parameter is the direction and speed of gravity.  We'll set it to
138:        (0, -10, 0), which approximates to realistic gravity, assuming that our
139:        units are metres. You could set it to (0,0,0) to disable gravity. And the
140:        last value is just a translation: Without this, the ellipsoid with which
141:        collision detection is done would be around the camera, and the camera would
142:        be in the middle of the ellipsoid. But as human beings, we are used to have our
143:        eyes on top of the body, with which we collide with our world, not in
144:        the middle of it. So we place the scene node 50 units over the center
145:        of the ellipsoid with this parameter. And that's it, collision
146:        detection works now.
147:      */
148: 
149:     // Set a jump speed of 3 units per second, which gives a fairly realistic jump
150:     // when used with the gravity of (0, -10, 0) in the collision response animator.
151:     scene::ICameraSceneNode *camera =
152:         smgr->addCameraSceneNodeFPS(0, 100.0f, .3f, ID_IsNotPickable, 0, 0, true, 3.f);
153:     camera->setPosition(core::vector3df(50, 50, -60));
154:     camera->setTarget(core::vector3df(-70, 30, -60));
155: 
156:     if (selector)
157:     {
158:         scene::ISceneNodeAnimator *anim = smgr->createCollisionResponseAnimator(
159:             selector, camera, core::vector3df(30, 50, 30),
160:             core::vector3df(0, -10, 0), core::vector3df(0, 30, 0));
161:         selector->drop();         // As soon as we're done with the selector, drop it.
162:         camera->addAnimator(anim);
163:         anim->drop();          // And likewise, drop the animator when we're done referring to it.
164:     }
165: 
166:     // Now I create three animated characters which we can pick, a dynamic light for
167:     // lighting them, and a billboard for drawing where we found an intersection.
168: 
169:     // First, let's get rid of the mouse cursor.  We'll use a billboard to show
170:     // what we're looking at.
171:     device->getCursorControl()->setVisible(false);
172: 
173:     // Add the billboard.
174:     scene::IBillboardSceneNode *bill = smgr->addBillboardSceneNode();
175:     bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
176:     bill->setMaterialTexture(0, driver->getTexture("../../media/particle.bmp"));
177:     bill->setMaterialFlag(video::EMF_LIGHTING, false);
178:     bill->setMaterialFlag(video::EMF_ZBUFFER, false);
179:     bill->setSize(core::dimension2d<f32>(20.0f, 20.0f));
180:     bill->setID(ID_IsNotPickable);     // This ensures that we don't accidentally ray-pick it
181: 
182:     /* Add 3 animated hominids, which we can pick using a ray-triangle intersection.
183:        They all animate quite slowly, to make it easier to see that accurate triangle
184:        selection is being performed. */
185:     scene::IAnimatedMeshSceneNode *node = 0;
186: 
187:     video::SMaterial material;
188: 
189:     // Add an MD2 node, which uses vertex-based animation.
190:     node = smgr->addAnimatedMeshSceneNode(smgr->getMesh("../../media/faerie.md2"),
191:                                           0, IDFlag_IsPickable | IDFlag_IsHighlightable);
192:     node->setPosition(core::vector3df(-90, -15, -140));   // Put its feet on the floor.
193:     node->setScale(core::vector3df(1.6f));     // Make it appear realistically scaled
194:     node->setMD2Animation(scene::EMAT_POINT);
195:     node->setAnimationSpeed(20.f);
196:     material.setTexture(0, driver->getTexture("../../media/faerie2.bmp"));
197:     material.Lighting         = true;
198:     material.NormalizeNormals = true;
199:     node->getMaterial(0)      = material;
200: 
201:     // Now create a triangle selector for it.  The selector will know that it
202:     // is associated with an animated node, and will update itself as necessary.
203:     selector = smgr->createTriangleSelector(node);
204:     node->setTriangleSelector(selector);
205:     selector->drop();     // We're done with this selector, so drop it now.
206: 
207:     // And this B3D file uses skinned skeletal animation.
208:     node = smgr->addAnimatedMeshSceneNode(smgr->getMesh("../../media/ninja.b3d"),
209:                                           0, IDFlag_IsPickable | IDFlag_IsHighlightable);
210:     node->setScale(core::vector3df(10));
211:     node->setPosition(core::vector3df(-75, -66, -80));
212:     node->setRotation(core::vector3df(0, 90, 0));
213:     node->setAnimationSpeed(8.f);
214:     node->getMaterial(0).NormalizeNormals = true;
215:     node->getMaterial(0).Lighting         = true;
216:     // Just do the same as we did above.
217:     selector = smgr->createTriangleSelector(node);
218:     node->setTriangleSelector(selector);
219:     selector->drop();
220: 
221:     // This X files uses skeletal animation, but without skinning.
222:     node = smgr->addAnimatedMeshSceneNode(smgr->getMesh("../../media/dwarf.x"),
223:                                           0, IDFlag_IsPickable | IDFlag_IsHighlightable);
224:     node->setPosition(core::vector3df(-70, -66, -30));   // Put its feet on the floor.
225:     node->setRotation(core::vector3df(0, -90, 0));   // And turn it towards the camera.
226:     node->setAnimationSpeed(20.f);
227:     node->getMaterial(0).Lighting = true;
228:     selector                      = smgr->createTriangleSelector(node);
229:     node->setTriangleSelector(selector);
230:     selector->drop();
231: 
232: 
233:     // And this mdl file uses skinned skeletal animation.
234:     node = smgr->addAnimatedMeshSceneNode(smgr->getMesh("../../media/yodan.mdl"),
235:                                           0, IDFlag_IsPickable | IDFlag_IsHighlightable);
236:     node->setPosition(core::vector3df(-90, -25, 20));
237:     node->setScale(core::vector3df(0.8f));
238:     node->getMaterial(0).Lighting = true;
239:     node->setAnimationSpeed(20.f);
240: 
241:     // Just do the same as we did above.
242:     selector = smgr->createTriangleSelector(node);
243:     node->setTriangleSelector(selector);
244:     selector->drop();
245: 
246:     material.setTexture(0, 0);
247:     material.Lighting = false;
248: 
249:     // Add a light, so that the unselected nodes aren't completely dark.
250:     scene::ILightSceneNode *light = smgr->addLightSceneNode(0, core::vector3df(-60, 100, 400),
251:                                                             video::SColorf(1.0f, 1.0f, 1.0f, 1.0f), 600.0f);
252:     light->setID(ID_IsNotPickable);     // Make it an invalid target for selection.
253: 
254:     // Remember which scene node is highlighted
255:     scene::ISceneNode             *highlightedSceneNode = 0;
256:     scene::ISceneCollisionManager *collMan              = smgr->getSceneCollisionManager();
257:     int                           lastFPS               = -1;
258: 
259:     // draw the selection triangle only as wireframe
260:     material.Wireframe = true;
261: 
262:     while (device->run())
263:         if (device->isWindowActive())
264:         {
265:             driver->beginScene(true, true, 0);
266:             smgr->drawAll();
267: 
268:             // Unlight any currently highlighted scene node
269:             if (highlightedSceneNode)
270:             {
271:                 highlightedSceneNode->setMaterialFlag(video::EMF_LIGHTING, true);
272:                 highlightedSceneNode = 0;
273:             }
274: 
275:             // All intersections in this example are done with a ray cast out from the camera to
276:             // a distance of 1000.  You can easily modify this to check (e.g.) a bullet
277:             // trajectory or a sword's position, or create a ray from a mouse click position using
278:             // ISceneCollisionManager::getRayFromScreenCoordinates()
279:             core::line3d<f32> ray;
280:             ray.start = camera->getPosition();
281:             ray.end   = ray.start + (camera->getTarget() - ray.start).normalize() * 1000.0f;
282: 
283:             // Tracks the current intersection point with the level or a mesh
284:             core::vector3df intersection;
285:             // Used to show with triangle has been hit
286:             core::triangle3df hitTriangle;
287: 
288:             // This call is all you need to perform ray/triangle collision on every scene node
289:             // that has a triangle selector, including the Quake level mesh.  It finds the nearest
290:             // collision point/triangle, and returns the scene node containing that point.
291:             // Irrlicht provides other types of selection, including ray/triangle selector,
292:             // ray/box and ellipse/triangle selector, plus associated helpers.
293:             // See the methods of ISceneCollisionManager
294:             scene::ISceneNode *selectedSceneNode =
295:                 collMan->getSceneNodeAndCollisionPointFromRay(
296:                     ray,
297:                     intersection,                     // This will be the position of the collision
298:                     hitTriangle,                     // This will be the triangle hit in the collision
299:                     IDFlag_IsPickable,                     // This ensures that only nodes that we have
300:                                                            // set up to be pickable are considered
301:                     0);                     // Check the entire scene (this is actually the implicit default)
302: 
303:             // If the ray hit anything, move the billboard to the collision position
304:             // and draw the triangle that was hit.
305:             if (selectedSceneNode)
306:             {
307:                 bill->setPosition(intersection);
308: 
309:                 // We need to reset the transform before doing our own rendering.
310:                 driver->setTransform(video::ETS_WORLD, core::matrix4());
311:                 driver->setMaterial(material);
312:                 driver->draw3DTriangle(hitTriangle, video::SColor(0, 255, 0, 0));
313: 
314:                 // We can check the flags for the scene node that was hit to see if it should be
315:                 // highlighted. The animated nodes can be highlighted, but not the Quake level mesh
316:                 if ((selectedSceneNode->getID() & IDFlag_IsHighlightable) == IDFlag_IsHighlightable)
317:                 {
318:                     highlightedSceneNode = selectedSceneNode;
319: 
320:                     // Highlighting in this case means turning lighting OFF for this node,
321:                     // which means that it will be drawn with full brightness.
322:                     highlightedSceneNode->setMaterialFlag(video::EMF_LIGHTING, false);
323:                 }
324:             }
325: 
326:             // We're all done drawing, so end the scene.
327:             driver->endScene();
328: 
329: #if CONFORM_TEST_DX9
330:             video::IImage *image = device->getVideoDriver()->createScreenShot();
331:             if (image)
332:             {
333:                 device->getVideoDriver()->writeImageToFile(image, "screenshot.bmp");
334:                 image->drop();
335:             }
336:             break;
337: #elif CONFORM_TEST_DX11
338:             video::IImage *image = device->getVideoDriver()->createScreenShot();
339:             if (image)
340:             {
341:                 device->getVideoDriver()->writeImageToFile(image, "screenshot.bmp");
342:                 image->drop();
343:             }
344:             break;
345: #endif
346: 
347:             int fps = driver->getFPS();
348: 
349:             if (lastFPS != fps)
350:             {
351:                 core::stringw str = L"Collision detection example - Irrlicht Engine [";
352:                 str += driver->getName();
353:                 str += "] FPS:";
354:                 str += fps;
355: 
356:                 device->setWindowCaption(str.c_str());
357:                 lastFPS = fps;
358:             }
359:         }
360: 
361:     device->drop();
362: 
363:     return 0;
364: }
365: 
366: /*
367: **/

(End of file - total 367 lines)
</content>

<system-reminder>
Instructions from: C:\Development\Graphics\irrlicht-1.8.3\examples\AGENTS.md
# EXAMPLES KNOWLEDGE BASE

## OVERVIEW
Tutorials and demo projects demonstrating Irrlicht features.

## WHERE TO LOOK
| Example | Feature |
|------|----------|
| `01.HelloWorld` | Basic engine setup |
| `02.Quake3Map` | Load Q3 level |
| `07.Collision` | Collision detection |
| `10.Shaders` | High-level shaders |
| `Demo` | Comprehensive tech demo |

## CONVENTIONS
- Each example is a standalone directory with its own `main.cpp`.
- Multi-platform Makefiles provided for each example.
- Common entry point: `createDevice`.
- Event handling via custom `IEventReceiver` implementations.

## BUILD
- Run `make` in an example directory for Linux.
- Use `all_macos` target for macOS.

</system-reminder>
```

**Tool: grep**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\include",
  "pattern": "cameraPosition|viewMatrix|getViewMatrix|getCameraPosition"
}
```

**Output:**
```
Found 6 matches
C:\Development\Graphics\irrlicht-1.8.3\include\SViewFrustum.h:
  Line 103:             core::vector3df cameraPosition;

  Line 130:             cameraPosition = other.cameraPosition;

  Line 153:             mat.transformVect(cameraPosition);

  Line 253:             boundingBox.reset (cameraPosition);


C:\Development\Graphics\irrlicht-1.8.3\include\ICameraSceneNode.h:
  Line 53:             virtual const core::matrix4&getViewMatrix() const = 0;

  Line 64:             virtual const core::matrix4&getViewMatrixAffector() const = 0;

```

---

## Assistant (Build · MiniMax-M2.7 · 17.7s)

_Thinking:_

让我分析cameraPosition和viewMatrix在Scene中是如何被计算得到的。我需要查看：

1. `addCameraSceneNodeFPS` - 这是创建相机的地方
2. `ICameraSceneNode` - 相机接口
3. `SViewFrustum` - 包含cameraPosition的结构
4. 实际的相机实现

让我先看看ICameraSceneNode接口和实际实现


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\include\\ICameraSceneNode.h"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\include\ICameraSceneNode.h</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #ifndef __I_CAMERA_SCENE_NODE_H_INCLUDED__
6: #define __I_CAMERA_SCENE_NODE_H_INCLUDED__
7: 
8: #include "ISceneNode.h"
9: #include "IEventReceiver.h"
10: 
11: namespace irr
12: {
13:     namespace scene
14:     {
15:         struct SViewFrustum;
16: 
17:         //! Scene Node which is a (controlable) camera.
18:         /** The whole scene will be rendered from the cameras point of view.
19:          * Because the ICameraScenNode is a SceneNode, it can be attached to any
20:          * other scene node, and will follow its parents movement, rotation and so
21:          * on.
22:          */
23:         class ICameraSceneNode : public ISceneNode, public IEventReceiver
24:         {
25: public:
26: 
27:             //! Constructor
28:             ICameraSceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,
29:                 const core::vector3df &position = core::vector3df(0, 0, 0),
30:                 const core::vector3df &rotation = core::vector3df(0, 0, 0),
31:                 const core::vector3df &scale = core::vector3df(1.0f, 1.0f, 1.0f))
32:                 : ISceneNode(parent, mgr, id, position, rotation, scale), IsOrthogonal(false) {}
33: 
34:             //! Sets the projection matrix of the camera.
35:             /** The core::matrix4 class has some methods to build a
36:              * projection matrix. e.g:
37:              * core::matrix4::buildProjectionMatrixPerspectiveFovLH.
38:              * Note that the matrix will only stay as set by this method until
39:              * one of the following Methods are called: setNearValue,
40:              * setFarValue, setAspectRatio, setFOV.
41:              * \param projection The new projection matrix of the camera.
42:              * \param isOrthogonal Set this to true if the matrix is an
43:              * orthogonal one (e.g. from matrix4::buildProjectionMatrixOrtho).
44:              */
45:             virtual void setProjectionMatrix(const core::matrix4 &projection, bool isOrthogonal = false) = 0;
46: 
47:             //! Gets the current projection matrix of the camera.
48:             /** \return The current projection matrix of the camera. */
49:             virtual const core::matrix4&getProjectionMatrix() const = 0;
50: 
51:             //! Gets the current view matrix of the camera.
52:             /** \return The current view matrix of the camera. */
53:             virtual const core::matrix4&getViewMatrix() const = 0;
54: 
55:             //! Sets a custom view matrix affector.
56:             /** The matrix passed here, will be multiplied with the view
57:              * matrix when it gets updated. This allows for custom camera
58:              * setups like, for example, a reflection camera.
59:              * \param affector The affector matrix. */
60:             virtual void setViewMatrixAffector(const core::matrix4 &affector) = 0;
61: 
62:             //! Get the custom view matrix affector.
63:             /** \return The affector matrix. */
64:             virtual const core::matrix4&getViewMatrixAffector() const = 0;
65: 
66:             //! It is possible to send mouse and key events to the camera.
67:             /** Most cameras may ignore this input, but camera scene nodes
68:              * which are created for example with
69:              * ISceneManager::addCameraSceneNodeMaya or
70:              * ISceneManager::addCameraSceneNodeFPS, may want to get
71:              * this input for changing their position, look at target or
72:              * whatever. */
73:             virtual bool OnEvent(const SEvent &event) = 0;
74: 
75:             //! Sets the look at target of the camera
76:             /** If the camera's target and rotation are bound ( @see
77:              * bindTargetAndRotation() ) then calling this will also change
78:              * the camera's scene node rotation to match the target.
79:              * Note that setTarget uses the current absolute position
80:              * internally, so if you changed setPosition since last rendering you must
81:              * call updateAbsolutePosition before using this function.
82:              * \param pos Look at target of the camera, in world co-ordinates. */
83:             virtual void setTarget(const core::vector3df &pos) = 0;
84: 
85:             //! Sets the rotation of the node.
86:             /** This only modifies the relative rotation of the node.
87:              * If the camera's target and rotation are bound ( @see
88:              * bindTargetAndRotation() ) then calling this will also change
89:              * the camera's target to match the rotation.
90:              * \param rotation New rotation of the node in degrees. */
91:             virtual void setRotation(const core::vector3df &rotation) = 0;
92: 
93:             //! Gets the current look at target of the camera
94:             /** \return The current look at target of the camera, in world co-ordinates */
95:             virtual const core::vector3df&getTarget() const = 0;
96: 
97:             //! Sets the up vector of the camera.
98:             /** \param pos: New upvector of the camera. */
99:             virtual void setUpVector(const core::vector3df &pos) = 0;
100: 
101:             //! Gets the up vector of the camera.
102:             /** \return The up vector of the camera, in world space. */
103:             virtual const core::vector3df&getUpVector() const = 0;
104: 
105:             //! Gets the value of the near plane of the camera.
106:             /** \return The value of the near plane of the camera. */
107:             virtual f32 getNearValue() const = 0;
108: 
109:             //! Gets the value of the far plane of the camera.
110:             /** \return The value of the far plane of the camera. */
111:             virtual f32 getFarValue() const = 0;
112: 
113:             //! Gets the aspect ratio of the camera.
114:             /** \return The aspect ratio of the camera. */
115:             virtual f32 getAspectRatio() const = 0;
116: 
117:             //! Gets the field of view of the camera.
118:             /** \return The field of view of the camera in radians. */
119:             virtual f32 getFOV() const = 0;
120: 
121:             //! Sets the value of the near clipping plane. (default: 1.0f)
122:             /** \param zn: New z near value. */
123:             virtual void setNearValue(f32 zn) = 0;
124: 
125:             //! Sets the value of the far clipping plane (default: 2000.0f)
126:             /** \param zf: New z far value. */
127:             virtual void setFarValue(f32 zf) = 0;
128: 
129:             //! Sets the aspect ratio (default: 4.0f / 3.0f)
130:             /** \param aspect: New aspect ratio. */
131:             virtual void setAspectRatio(f32 aspect) = 0;
132: 
133:             //! Sets the field of view (Default: PI / 2.5f)
134:             /** \param fovy: New field of view in radians. */
135:             virtual void setFOV(f32 fovy) = 0;
136: 
137:             //! Get the view frustum.
138:             /** Needed sometimes by bspTree or LOD render nodes.
139:              * \return The current view frustum. */
140:             virtual const SViewFrustum* getViewFrustum() const = 0;
141: 
142:             //! Disables or enables the camera to get key or mouse inputs.
143:             /** If this is set to true, the camera will respond to key
144:              * inputs otherwise not. */
145:             virtual void setInputReceiverEnabled(bool enabled) = 0;
146: 
147:             //! Checks if the input receiver of the camera is currently enabled.
148:             virtual bool isInputReceiverEnabled() const = 0;
149: 
150:             //! Checks if a camera is orthogonal.
151:             virtual bool isOrthogonal() const
152:             {
153:                 _IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
154:                 return IsOrthogonal;
155:             }
156: 
157:             //! Binds the camera scene node's rotation to its target position and vice vera, or unbinds them.
158:             /** When bound, calling setRotation() will update the camera's
159:              * target position to be along its +Z axis, and likewise calling
160:              * setTarget() will update its rotation so that its +Z axis will
161:              * point at the target point. FPS camera use this binding by
162:              * default; other cameras do not.
163:              * \param bound True to bind the camera's scene node rotation
164:              * and targetting, false to unbind them.
165:              * @see getTargetAndRotationBinding() */
166:             virtual void bindTargetAndRotation(bool bound) = 0;
167: 
168:             //! Queries if the camera scene node's rotation and its target position are bound together.
169:             /** @see bindTargetAndRotation() */
170:             virtual bool getTargetAndRotationBinding(void) const = 0;
171: 
172:             //! Writes attributes of the camera node
173:             virtual void serializeAttributes(io::IAttributes *out, io::SAttributeReadWriteOptions *options = 0) const
174:             {
175:                 ISceneNode::serializeAttributes(out, options);
176: 
177:                 if (!out)
178:                     return;
179: 
180:                 out->addBool    ("IsOrthogonal", IsOrthogonal);
181:             }
182: 
183:             //! Reads attributes of the camera node
184:             virtual void deserializeAttributes(io::IAttributes *in, io::SAttributeReadWriteOptions *options = 0)
185:             {
186:                 ISceneNode::deserializeAttributes(in, options);
187: 
188:                 if (!in)
189:                     return;
190: 
191:                 if (in->findAttribute("IsOrthogonal"))
192:                     IsOrthogonal = in->getAttributeAsBool("IsOrthogonal");
193:             }
194: 
195: protected:
196: 
197:             void cloneMembers(ICameraSceneNode *toCopyFrom)
198:             {
199:                 IsOrthogonal = toCopyFrom->IsOrthogonal;
200:             }
201: 
202:             bool IsOrthogonal;
203:         };
204:     } // end namespace scene
205: } // end namespace irr
206: #endif

(End of file - total 206 lines)
</content>

<system-reminder>
Instructions from: C:\Development\Graphics\irrlicht-1.8.3\include\AGENTS.md
# PUBLIC API KNOWLEDGE BASE

## OVERVIEW
Public engine headers defining the stable Irrlicht API.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Core Entry | `irrlicht.h` | Include this in applications |
| Video | `IVideoDriver.h` | 2D/3D rendering interface |
| Scene | `ISceneManager.h`| Main scene graph entry |
| GUI | `IGUIEnvironment.h`| Main GUI management |
| IO | `IFileSystem.h` | File and archive access |

## CONVENTIONS
- Interfaces prefixed with `I` (e.g., `IVideoDriver`).
- Most classes inherit from `IReferenceCounted`.
- Heavy use of template classes for math and containers.
- Documentation usually provided as Doxygen comments in headers.

## USAGE
- Use `createDevice` from `irrlicht.h` to start.
- Always `drop()` objects created with `create...` or `add...` when done.

</system-reminder>
```

**Tool: grep**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht",
  "pattern": "CCameraSceneNode|addCameraSceneNodeFPS"
}
```

**Output:**
```
Found 82 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht.2019.vcxproj:
  Line 1820:     <ClInclude Include="CCameraSceneNode.h" />

  Line 2071:     <ClCompile Include="CCameraSceneNode.cpp" />


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht.2019.vcxproj.filters:
  Line 675:     <ClInclude Include="CCameraSceneNode.h">

  Line 1439:     <ClCompile Include="CCameraSceneNode.cpp">


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CCameraSceneNode.cpp:
  Line 5: #include "CCameraSceneNode.h"

  Line 15:         CCameraSceneNode::CCameraSceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,

  Line 22:             setDebugName("CCameraSceneNode");

  Line 41:         void CCameraSceneNode::setInputReceiverEnabled(bool enabled)

  Line 48:         bool CCameraSceneNode::isInputReceiverEnabled() const

  Line 59:         void CCameraSceneNode::setProjectionMatrix(const core::matrix4 &projection, bool isOrthogonal)

  Line 68:         const core::matrix4&CCameraSceneNode::getProjectionMatrix() const

  Line 76:         const core::matrix4&CCameraSceneNode::getViewMatrix() const

  Line 86:         void CCameraSceneNode::setViewMatrixAffector(const core::matrix4 &affector)

  Line 93:         const core::matrix4&CCameraSceneNode::getViewMatrixAffector() const

  Line 104:         bool CCameraSceneNode::OnEvent(const SEvent &event)

  Line 124:         void CCameraSceneNode::setTarget(const core::vector3df &pos)

  Line 141:         void CCameraSceneNode::setRotation(const core::vector3df &rotation)

  Line 152:         const core::vector3df&CCameraSceneNode::getTarget() const

  Line 160:         void CCameraSceneNode::setUpVector(const core::vector3df &pos)

  Line 168:         const core::vector3df&CCameraSceneNode::getUpVector() const

  Line 174:         f32 CCameraSceneNode::getNearValue() const

  Line 180:         f32 CCameraSceneNode::getFarValue() const

  Line 186:         f32 CCameraSceneNode::getAspectRatio() const

  Line 192:         f32 CCameraSceneNode::getFOV() const

  Line 198:         void CCameraSceneNode::setNearValue(f32 f)

  Line 205:         void CCameraSceneNode::setFarValue(f32 f)

  Line 212:         void CCameraSceneNode::setAspectRatio(f32 f)

  Line 219:         void CCameraSceneNode::setFOV(f32 f)

  Line 226:         void CCameraSceneNode::recalculateProjectionMatrix()

  Line 233:         void CCameraSceneNode::OnRegisterSceneNode()

  Line 243:         void CCameraSceneNode::render()

  Line 276:         const core::aabbox3d<f32>&CCameraSceneNode::getBoundingBox() const

  Line 283:         const SViewFrustum* CCameraSceneNode::getViewFrustum() const

  Line 289:         void CCameraSceneNode::recalculateViewArea()

  Line 301:         void CCameraSceneNode::serializeAttributes(io::IAttributes *out, io::SAttributeReadWriteOptions *options) const

  Line 316:         void CCameraSceneNode::deserializeAttributes(io::IAttributes *in, io::SAttributeReadWriteOptions *options)

  Line 336:         void CCameraSceneNode::bindTargetAndRotation(bool bound)

  Line 343:         bool CCameraSceneNode::getTargetAndRotationBinding(void) const

  Line 350:         ISceneNode* CCameraSceneNode::clone(ISceneNode *newParent, ISceneManager *newManager)

  Line 360:             CCameraSceneNode *nb = new CCameraSceneNode(newParent,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSceneNodeAnimatorCameraFPS.cpp:
  Line 27:             setDebugName("CCameraSceneNodeAnimatorFPS");


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CDefaultSceneNodeFactory.cpp:
  Line 116:                     return Manager->addCameraSceneNodeFPS(parent);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSceneManager.cpp:
  Line 139: #include "CCameraSceneNode.h"

  Line 703:             ICameraSceneNode *node = new CCameraSceneNode(parent, this, id, position, lookat);

  Line 739:         ICameraSceneNode* CSceneManager::addCameraSceneNodeFPS(ISceneNode *parent,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht10.0.vcxproj:
  Line 1038:     <ClInclude Include="CCameraSceneNode.h" />

  Line 1285:     <ClCompile Include="CCameraSceneNode.cpp" />


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\SConstruct:
  Line 30: IRROBJ = ['CBillboardSceneNode.cpp', 'CCameraSceneNode.cpp', 'CDummyTransformationSceneNode.cpp', 'CEmptySceneNode.cpp', 'CGeometryCreator.cpp', 'CLightSceneNode.cpp', 'CMeshManipulator.cpp', 'CMetaTriangleSelector.cpp', 'COctreeSceneNode.cpp', 'COctreeTriangleSelector.cpp', 'CSceneCollisionManager.cpp', 'CSceneManager.cpp', 'CShadowVolumeSceneNode.cpp', 'CSkyBoxSceneNode.cpp', 'CSkyDomeSceneNode.cpp', 'CTerrainSceneNode.cpp', 'CTerrainTriangleSelector.cpp', 'CVolumeLightSceneNode.cpp', 'CCubeSceneNode.cpp', 'CSphereSceneNode.cpp', 'CTextSceneNode.cpp', 'CTriangleBBSelector.cpp', 'CTriangleSelector.cpp', 'CWaterSurfaceSceneNode.cpp', 'CMeshCache.cpp', 'CDefaultSceneNodeAnimatorFactory.cpp', 'CDefaultSceneNodeFactory.cpp'];


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Makefile:
  Line 38: IRROBJ = CBillboardSceneNode.o CCameraSceneNode.o CDummyTransformationSceneNode.o CEmptySceneNode.o CGeometryCreator.o CLightSceneNode.o CMeshManipulator.o CMetaTriangleSelector.o COctreeSceneNode.o COctreeTriangleSelector.o CSceneCollisionManager.o CSceneManager.o CShadowVolumeSceneNode.o CSkyBoxSceneNode.o CSkyDomeSceneNode.o CTerrainSceneNode.o CTerrainTriangleSelector.o CVolumeLightSceneNode.o CCubeSceneNode.o CSphereSceneNode.o CTextSceneNode.o CTriangleBBSelector.o CTriangleSelector.o CWaterSurfaceSceneNode.o CMeshCache.o CDefaultSceneNodeAnimatorFactory.o CDefaultSceneNodeFactory.o CSceneLoaderIrr.o


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\MacOSX\MacOSX.xcodeproj\project.pbxproj:
  Line 391: 		4C53E4670A4856B30014E966 /* CCameraSceneNode.cpp in Sources */ = {isa = PBXBuildFile; fileRef = 4C53DEF80A484C220014E966 /* CCameraSceneNode.cpp */; };

  Line 848: 		95972A6512C192DA00BF73D3 /* CCameraSceneNode.cpp in Sources */ = {isa = PBXBuildFile; fileRef = 4C53DEF80A484C220014E966 /* CCameraSceneNode.cpp */; };

  Line 1576: 		4C53DEF80A484C220014E966 /* CCameraSceneNode.cpp */ = {isa = PBXFileReference; fileEncoding = 30; lastKnownFileType = sourcecode.cpp.cpp; path = CCameraSceneNode.cpp; sourceTree = "<group>"; };

  Line 1577: 		4C53DEF90A484C220014E966 /* CCameraSceneNode.h */ = {isa = PBXFileReference; fileEncoding = 30; lastKnownFileType = sourcecode.c.h; path = CCameraSceneNode.h; sourceTree = "<group>"; };

  Line 2823: 				4C53DEF80A484C220014E966 /* CCameraSceneNode.cpp */,

  Line 2824: 				4C53DEF90A484C220014E966 /* CCameraSceneNode.h */,

  Line 4772: 				95972A6512C192DA00BF73D3 /* CCameraSceneNode.cpp in Sources */,

  Line 5192: 				4C53E4670A4856B30014E966 /* CCameraSceneNode.cpp in Sources */,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht_xbox.vcproj:
  Line 722: 				RelativePath=".\CCameraSceneNode.cpp">

  Line 725: 				RelativePath=".\CCameraSceneNode.h">


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht_mobile6.vcproj:
  Line 1965: 					RelativePath="CCameraSceneNode.cpp"

  Line 1969: 					RelativePath="CCameraSceneNode.h"


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht9.0.vcproj:
  Line 1643: 						RelativePath="CCameraSceneNode.cpp"

  Line 1647: 						RelativePath="CCameraSceneNode.h"


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht8.0.vcproj:
  Line 2376: 					RelativePath=".\CCameraSceneNode.cpp"

  Line 2380: 					RelativePath=".\CCameraSceneNode.h"


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht12.0.vcxproj.filters:
  Line 672:     <ClInclude Include="CCameraSceneNode.h">

  Line 1418:     <ClCompile Include="CCameraSceneNode.cpp">


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht12.0.vcxproj:
  Line 1038:     <ClInclude Include="CCameraSceneNode.h" />

  Line 1285:     <ClCompile Include="CCameraSceneNode.cpp" />


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht11.0.vcxproj.filters:
  Line 672:     <ClInclude Include="CCameraSceneNode.h">

  Line 1418:     <ClCompile Include="CCameraSceneNode.cpp">


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht11.0.vcxproj:
  Line 1039:     <ClInclude Include="CCameraSceneNode.h" />

  Line 1286:     <ClCompile Include="CCameraSceneNode.cpp" />


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht.dev:
  Line 1893: FileName=CCameraSceneNode.cpp

  Line 1903: FileName=CCameraSceneNode.h


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht10.0.vcxproj.filters:
  Line 672:     <ClInclude Include="CCameraSceneNode.h">

  Line 1418:     <ClCompile Include="CCameraSceneNode.cpp">


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht-gcc.cbp:
  Line 632: 		<Unit filename="CCameraSceneNode.cpp" />

  Line 633: 		<Unit filename="CCameraSceneNode.h" />


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSceneManager.h:
  Line 149:             virtual ICameraSceneNode* addCameraSceneNodeFPS(ISceneNode *parent = 0,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CCameraSceneNode.h:
  Line 15:         class CCameraSceneNode : public ICameraSceneNode

  Line 20:             CCameraSceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,

```

---

## Assistant (Build · MiniMax-M2.7 · 10.6s)

_Thinking:_

让我查看CCameraSceneNode的实现来了解cameraPosition和viewMatrix是如何计算的。


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CCameraSceneNode.cpp"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CCameraSceneNode.cpp</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #include "CCameraSceneNode.h"
6: #include "ISceneManager.h"
7: #include "IVideoDriver.h"
8: #include "os.h"
9: 
10: namespace irr
11: {
12:     namespace scene
13:     {
14:         //! constructor
15:         CCameraSceneNode::CCameraSceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,
16:             const core::vector3df &position, const core::vector3df &lookat)
17:             : ICameraSceneNode(parent, mgr, id, position),
18:             Target(lookat), UpVector(0.0f, 1.0f, 0.0f), ZNear(1.0f), ZFar(3000.0f),
19:             InputReceiverEnabled(true), TargetAndRotationAreBound(false)
20:         {
21: #ifdef _DEBUG
22:             setDebugName("CCameraSceneNode");
23: #endif
24: 
25:             // set default projection
26:             Fovy = core::PI / 2.5f; // Field of view, in radians.
27: 
28:             const video::IVideoDriver* const d = mgr ? mgr->getVideoDriver() : 0;
29:             if (d)
30:                 Aspect = (f32)d->getCurrentRenderTargetSize().Width /
31:                     (f32)d->getCurrentRenderTargetSize().Height;
32:             else
33:                 Aspect = 4.0f / 3.0f; // Aspect ratio.
34: 
35:             recalculateProjectionMatrix();
36:             recalculateViewArea();
37:         }
38: 
39: 
40:         //! Disables or enables the camera to get key or mouse inputs.
41:         void CCameraSceneNode::setInputReceiverEnabled(bool enabled)
42:         {
43:             InputReceiverEnabled = enabled;
44:         }
45: 
46: 
47:         //! Returns if the input receiver of the camera is currently enabled.
48:         bool CCameraSceneNode::isInputReceiverEnabled() const
49:         {
50:             _IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
51:             return InputReceiverEnabled;
52:         }
53: 
54: 
55:         //! Sets the projection matrix of the camera.
56:         /** The core::matrix4 class has some methods
57:          * to build a projection matrix. e.g: core::matrix4::buildProjectionMatrixPerspectiveFovLH
58:          * \param projection: The new projection matrix of the camera. */
59:         void CCameraSceneNode::setProjectionMatrix(const core::matrix4 &projection, bool isOrthogonal)
60:         {
61:             IsOrthogonal                                  = isOrthogonal;
62:             ViewArea.getTransform (video::ETS_PROJECTION) = projection;
63:         }
64: 
65: 
66:         //! Gets the current projection matrix of the camera
67:         //! \return Returns the current projection matrix of the camera.
68:         const core::matrix4&CCameraSceneNode::getProjectionMatrix() const
69:         {
70:             return ViewArea.getTransform (video::ETS_PROJECTION);
71:         }
72: 
73: 
74:         //! Gets the current view matrix of the camera
75:         //! \return Returns the current view matrix of the camera.
76:         const core::matrix4&CCameraSceneNode::getViewMatrix() const
77:         {
78:             return ViewArea.getTransform (video::ETS_VIEW);
79:         }
80: 
81: 
82:         //! Sets a custom view matrix affector. The matrix passed here, will be
83:         //! multiplied with the view matrix when it gets updated.
84:         //! This allows for custom camera setups like, for example, a reflection camera.
85:         /** \param affector: The affector matrix. */
86:         void CCameraSceneNode::setViewMatrixAffector(const core::matrix4 &affector)
87:         {
88:             Affector = affector;
89:         }
90: 
91: 
92:         //! Gets the custom view matrix affector.
93:         const core::matrix4&CCameraSceneNode::getViewMatrixAffector() const
94:         {
95:             return Affector;
96:         }
97: 
98: 
99:         //! It is possible to send mouse and key events to the camera. Most cameras
100:         //! may ignore this input, but camera scene nodes which are created for
101:         //! example with scene::ISceneManager::addMayaCameraSceneNode or
102:         //! scene::ISceneManager::addFPSCameraSceneNode, may want to get this input
103:         //! for changing their position, look at target or whatever.
104:         bool CCameraSceneNode::OnEvent(const SEvent &event)
105:         {
106:             if (!InputReceiverEnabled)
107:                 return false;
108: 
109:             // send events to event receiving animators
110: 
111:             ISceneNodeAnimatorList::Iterator ait = Animators.begin();
112: 
113:             for (; ait != Animators.end(); ++ait)
114:                 if ((*ait)->isEventReceiverEnabled() && (*ait)->OnEvent(event))
115:                     return true;
116: 
117:             // if nobody processed the event, return false
118:             return false;
119:         }
120: 
121: 
122:         //! sets the look at target of the camera
123:         //! \param pos: Look at target of the camera.
124:         void CCameraSceneNode::setTarget(const core::vector3df &pos)
125:         {
126:             Target = pos;
127: 
128:             if (TargetAndRotationAreBound)
129:             {
130:                 const core::vector3df toTarget = Target - getAbsolutePosition();
131:                 ISceneNode::setRotation(toTarget.getHorizontalAngle());
132:             }
133:         }
134: 
135: 
136:         //! Sets the rotation of the node.
137:         /** This only modifies the relative rotation of the node.
138:         * If the camera's target and rotation are bound ( @see bindTargetAndRotation() )
139:         * then calling this will also change the camera's target to match the rotation.
140:         * \param rotation New rotation of the node in degrees. */
141:         void CCameraSceneNode::setRotation(const core::vector3df &rotation)
142:         {
143:             if (TargetAndRotationAreBound)
144:                 Target = getAbsolutePosition() + rotation.rotationToDirection();
145: 
146:             ISceneNode::setRotation(rotation);
147:         }
148: 
149: 
150:         //! Gets the current look at target of the camera
151:         //! \return Returns the current look at target of the camera
152:         const core::vector3df&CCameraSceneNode::getTarget() const
153:         {
154:             return Target;
155:         }
156: 
157: 
158:         //! sets the up vector of the camera
159:         //! \param pos: New upvector of the camera.
160:         void CCameraSceneNode::setUpVector(const core::vector3df &pos)
161:         {
162:             UpVector = pos;
163:         }
164: 
165: 
166:         //! Gets the up vector of the camera.
167:         //! \return Returns the up vector of the camera.
168:         const core::vector3df&CCameraSceneNode::getUpVector() const
169:         {
170:             return UpVector;
171:         }
172: 
173: 
174:         f32 CCameraSceneNode::getNearValue() const
175:         {
176:             return ZNear;
177:         }
178: 
179: 
180:         f32 CCameraSceneNode::getFarValue() const
181:         {
182:             return ZFar;
183:         }
184: 
185: 
186:         f32 CCameraSceneNode::getAspectRatio() const
187:         {
188:             return Aspect;
189:         }
190: 
191: 
192:         f32 CCameraSceneNode::getFOV() const
193:         {
194:             return Fovy;
195:         }
196: 
197: 
198:         void CCameraSceneNode::setNearValue(f32 f)
199:         {
200:             ZNear = f;
201:             recalculateProjectionMatrix();
202:         }
203: 
204: 
205:         void CCameraSceneNode::setFarValue(f32 f)
206:         {
207:             ZFar = f;
208:             recalculateProjectionMatrix();
209:         }
210: 
211: 
212:         void CCameraSceneNode::setAspectRatio(f32 f)
213:         {
214:             Aspect = f;
215:             recalculateProjectionMatrix();
216:         }
217: 
218: 
219:         void CCameraSceneNode::setFOV(f32 f)
220:         {
221:             Fovy = f;
222:             recalculateProjectionMatrix();
223:         }
224: 
225: 
226:         void CCameraSceneNode::recalculateProjectionMatrix()
227:         {
228:             ViewArea.getTransform (video::ETS_PROJECTION).buildProjectionMatrixPerspectiveFovLH(Fovy, Aspect, ZNear, ZFar);
229:         }
230: 
231: 
232:         //! prerender
233:         void CCameraSceneNode::OnRegisterSceneNode()
234:         {
235:             if (SceneManager->getActiveCamera () == this)
236:                 SceneManager->registerNodeForRendering(this, ESNRP_CAMERA);
237: 
238:             ISceneNode::OnRegisterSceneNode();
239:         }
240: 
241: 
242:         //! render
243:         void CCameraSceneNode::render()
244:         {
245:             core::vector3df pos  = getAbsolutePosition();
246:             core::vector3df tgtv = Target - pos;
247: 
248:             tgtv.normalize();
249: 
250:             // if upvector and vector to the target are the same, we have a
251:             // problem. so solve this problem:
252:             core::vector3df up = UpVector;
253:             up.normalize();
254: 
255:             f32 dp = tgtv.dotProduct(up);
256: 
257:             if (core::equals(core::abs_<f32>(dp), 1.f))
258:             {
259:                 up.X += 0.5f;
260:             }
261: 
262:             ViewArea.getTransform(video::ETS_VIEW).buildCameraLookAtMatrixLH(pos, Target, up);
263:             ViewArea.getTransform(video::ETS_VIEW) *= Affector;
264:             recalculateViewArea();
265: 
266:             video::IVideoDriver *driver = SceneManager->getVideoDriver();
267:             if (driver)
268:             {
269:                 driver->setTransform(video::ETS_PROJECTION, ViewArea.getTransform (video::ETS_PROJECTION));
270:                 driver->setTransform(video::ETS_VIEW, ViewArea.getTransform (video::ETS_VIEW));
271:             }
272:         }
273: 
274: 
275:         //! returns the axis aligned bounding box of this node
276:         const core::aabbox3d<f32>&CCameraSceneNode::getBoundingBox() const
277:         {
278:             return ViewArea.getBoundingBox();
279:         }
280: 
281: 
282:         //! returns the view frustum. needed sometimes by bsp or lod render nodes.
283:         const SViewFrustum* CCameraSceneNode::getViewFrustum() const
284:         {
285:             return &ViewArea;
286:         }
287: 
288: 
289:         void CCameraSceneNode::recalculateViewArea()
290:         {
291:             ViewArea.cameraPosition = getAbsolutePosition();
292: 
293:             core::matrix4 m(core::matrix4::EM4CONST_NOTHING);
294:             m.setbyproduct_nocheck(ViewArea.getTransform(video::ETS_PROJECTION),
295:                 ViewArea.getTransform(video::ETS_VIEW));
296:             ViewArea.setFrom(m);
297:         }
298: 
299: 
300:         //! Writes attributes of the scene node.
301:         void CCameraSceneNode::serializeAttributes(io::IAttributes *out, io::SAttributeReadWriteOptions *options) const
302:         {
303:             ICameraSceneNode::serializeAttributes(out, options);
304: 
305:             out->addVector3d("Target", Target);
306:             out->addVector3d("UpVector", UpVector);
307:             out->addFloat("Fovy", Fovy);
308:             out->addFloat("Aspect", Aspect);
309:             out->addFloat("ZNear", ZNear);
310:             out->addFloat("ZFar", ZFar);
311:             out->addBool("Binding", TargetAndRotationAreBound);
312:             out->addBool("ReceiveInput", InputReceiverEnabled);
313:         }
314: 
315:         //! Reads attributes of the scene node.
316:         void CCameraSceneNode::deserializeAttributes(io::IAttributes *in, io::SAttributeReadWriteOptions *options)
317:         {
318:             ICameraSceneNode::deserializeAttributes(in, options);
319: 
320:             Target                    = in->getAttributeAsVector3d("Target");
321:             UpVector                  = in->getAttributeAsVector3d("UpVector");
322:             Fovy                      = in->getAttributeAsFloat("Fovy");
323:             Aspect                    = in->getAttributeAsFloat("Aspect");
324:             ZNear                     = in->getAttributeAsFloat("ZNear");
325:             ZFar                      = in->getAttributeAsFloat("ZFar");
326:             TargetAndRotationAreBound = in->getAttributeAsBool("Binding");
327:             if (in->findAttribute("ReceiveInput"))
328:                 InputReceiverEnabled = in->getAttributeAsBool("InputReceiverEnabled");
329: 
330:             recalculateProjectionMatrix();
331:             recalculateViewArea();
332:         }
333: 
334: 
335:         //! Set the binding between the camera's rotation adn target.
336:         void CCameraSceneNode::bindTargetAndRotation(bool bound)
337:         {
338:             TargetAndRotationAreBound = bound;
339:         }
340: 
341: 
342:         //! Gets the binding between the camera's rotation and target.
343:         bool CCameraSceneNode::getTargetAndRotationBinding(void) const
344:         {
345:             return TargetAndRotationAreBound;
346:         }
347: 
348: 
349:         //! Creates a clone of this scene node and its children.
350:         ISceneNode* CCameraSceneNode::clone(ISceneNode *newParent, ISceneManager *newManager)
351:         {
352:             ICameraSceneNode::clone(newParent, newManager);
353: 
354:             if (!newParent)
355:                 newParent = Parent;
356: 
357:             if (!newManager)
358:                 newManager = SceneManager;
359: 
360:             CCameraSceneNode *nb = new CCameraSceneNode(newParent,
361:                     newManager, ID, RelativeTranslation, Target);
362: 
363:             nb->ISceneNode::cloneMembers(this, newManager);
364:             nb->ICameraSceneNode::cloneMembers(this);
365: 
366:             nb->Target                    = Target;
367:             nb->UpVector                  = UpVector;
368:             nb->Fovy                      = Fovy;
369:             nb->Aspect                    = Aspect;
370:             nb->ZNear                     = ZNear;
371:             nb->ZFar                      = ZFar;
372:             nb->ViewArea                  = ViewArea;
373:             nb->Affector                  = Affector;
374:             nb->InputReceiverEnabled      = InputReceiverEnabled;
375:             nb->TargetAndRotationAreBound = TargetAndRotationAreBound;
376: 
377:             if (newParent)
378:                 nb->drop();
379: 
380:             return nb;
381:         }
382:     } // end namespace
383: } // end namespace

(End of file - total 383 lines)
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
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\include\\SViewFrustum.h"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\include\SViewFrustum.h</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #ifndef __S_VIEW_FRUSTUM_H_INCLUDED__
6: #define __S_VIEW_FRUSTUM_H_INCLUDED__
7: 
8: #include "plane3d.h"
9: #include "vector3d.h"
10: #include "line3d.h"
11: #include "aabbox3d.h"
12: #include "matrix4.h"
13: #include "IVideoDriver.h"
14: 
15: namespace irr
16: {
17:     namespace scene
18:     {
19:         //! Defines the view frustum. That's the space visible by the camera.
20:         /** The view frustum is enclosed by 6 planes. These six planes share
21:          * eight points. A bounding box around these eight points is also stored in
22:          * this structure.
23:          */
24:         struct SViewFrustum
25:         {
26:             enum VFPLANES
27:             {
28:                 //! Far plane of the frustum. That is the plane farest away from the eye.
29:                 VF_FAR_PLANE = 0,
30:                 //! Near plane of the frustum. That is the plane nearest to the eye.
31:                 VF_NEAR_PLANE,
32:                 //! Left plane of the frustum.
33:                 VF_LEFT_PLANE,
34:                 //! Right plane of the frustum.
35:                 VF_RIGHT_PLANE,
36:                 //! Bottom plane of the frustum.
37:                 VF_BOTTOM_PLANE,
38:                 //! Top plane of the frustum.
39:                 VF_TOP_PLANE,
40: 
41:                 //! Amount of planes enclosing the view frustum. Should be 6.
42:                 VF_PLANE_COUNT
43:             };
44: 
45: 
46:             //! Default Constructor
47:             SViewFrustum() {}
48: 
49:             //! Copy Constructor
50:             SViewFrustum(const SViewFrustum &other);
51: 
52:             //! This constructor creates a view frustum based on a projection and/or view matrix.
53:             SViewFrustum(const core::matrix4 &mat);
54: 
55:             //! This constructor creates a view frustum based on a projection and/or view matrix.
56:             inline void setFrom(const core::matrix4 &mat);
57: 
58:             //! transforms the frustum by the matrix
59:             /** \param mat: Matrix by which the view frustum is transformed.*/
60:             void transform(const core::matrix4 &mat);
61: 
62:             //! returns the point which is on the far left upper corner inside the the view frustum.
63:             core::vector3df getFarLeftUp() const;
64: 
65:             //! returns the point which is on the far left bottom corner inside the the view frustum.
66:             core::vector3df getFarLeftDown() const;
67: 
68:             //! returns the point which is on the far right top corner inside the the view frustum.
69:             core::vector3df getFarRightUp() const;
70: 
71:             //! returns the point which is on the far right bottom corner inside the the view frustum.
72:             core::vector3df getFarRightDown() const;
73: 
74:             //! returns the point which is on the near left upper corner inside the the view frustum.
75:             core::vector3df getNearLeftUp() const;
76: 
77:             //! returns the point which is on the near left bottom corner inside the the view frustum.
78:             core::vector3df getNearLeftDown() const;
79: 
80:             //! returns the point which is on the near right top corner inside the the view frustum.
81:             core::vector3df getNearRightUp() const;
82: 
83:             //! returns the point which is on the near right bottom corner inside the the view frustum.
84:             core::vector3df getNearRightDown() const;
85: 
86:             //! returns a bounding box enclosing the whole view frustum
87:             const core::aabbox3d<f32>&getBoundingBox() const;
88: 
89:             //! recalculates the bounding box member based on the planes
90:             inline void recalculateBoundingBox();
91: 
92:             //! get the given state's matrix based on frustum E_TRANSFORMATION_STATE
93:             core::matrix4&getTransform(video::E_TRANSFORMATION_STATE state);
94: 
95:             //! get the given state's matrix based on frustum E_TRANSFORMATION_STATE
96:             const core::matrix4&getTransform(video::E_TRANSFORMATION_STATE state) const;
97: 
98:             //! clips a line to the view frustum.
99:             /** \return True if the line was clipped, false if not */
100:             bool clipLine(core::line3d<f32> &line) const;
101: 
102:             //! the position of the camera
103:             core::vector3df cameraPosition;
104: 
105:             //! all planes enclosing the view frustum.
106:             core::plane3d<f32> planes[VF_PLANE_COUNT];
107: 
108:             //! bounding box around the view frustum
109:             core::aabbox3d<f32> boundingBox;
110: 
111: private:
112:             //! Hold a copy of important transform matrices
113:             enum E_TRANSFORMATION_STATE_FRUSTUM
114:             {
115:                 ETS_VIEW       = 0,
116:                 ETS_PROJECTION = 1,
117:                 ETS_COUNT_FRUSTUM
118:             };
119: 
120:             //! Hold a copy of important transform matrices
121:             core::matrix4 Matrices[ETS_COUNT_FRUSTUM];
122:         };
123: 
124: 
125:         /*!
126:          *  Copy constructor ViewFrustum
127:          */
128:         inline SViewFrustum::SViewFrustum(const SViewFrustum &other)
129:         {
130:             cameraPosition = other.cameraPosition;
131:             boundingBox    = other.boundingBox;
132: 
133:             u32 i;
134: 
135:             for (i = 0; i < VF_PLANE_COUNT; ++i)
136:                 planes[i] = other.planes[i];
137: 
138:             for (i = 0; i < ETS_COUNT_FRUSTUM; ++i)
139:                 Matrices[i] = other.Matrices[i];
140:         }
141: 
142:         inline SViewFrustum::SViewFrustum(const core::matrix4 &mat)
143:         {
144:             setFrom (mat);
145:         }
146: 
147: 
148:         inline void SViewFrustum::transform(const core::matrix4 &mat)
149:         {
150:             for (u32 i = 0; i < VF_PLANE_COUNT; ++i)
151:                 mat.transformPlane(planes[i]);
152: 
153:             mat.transformVect(cameraPosition);
154:             recalculateBoundingBox();
155:         }
156: 
157: 
158:         inline core::vector3df SViewFrustum::getFarLeftUp() const
159:         {
160:             core::vector3df p;
161: 
162:             planes[scene::SViewFrustum::VF_FAR_PLANE].getIntersectionWithPlanes(
163:                 planes[scene::SViewFrustum::VF_TOP_PLANE],
164:                 planes[scene::SViewFrustum::VF_LEFT_PLANE], p);
165: 
166:             return p;
167:         }
168: 
169:         inline core::vector3df SViewFrustum::getFarLeftDown() const
170:         {
171:             core::vector3df p;
172: 
173:             planes[scene::SViewFrustum::VF_FAR_PLANE].getIntersectionWithPlanes(
174:                 planes[scene::SViewFrustum::VF_BOTTOM_PLANE],
175:                 planes[scene::SViewFrustum::VF_LEFT_PLANE], p);
176: 
177:             return p;
178:         }
179: 
180:         inline core::vector3df SViewFrustum::getFarRightUp() const
181:         {
182:             core::vector3df p;
183: 
184:             planes[scene::SViewFrustum::VF_FAR_PLANE].getIntersectionWithPlanes(
185:                 planes[scene::SViewFrustum::VF_TOP_PLANE],
186:                 planes[scene::SViewFrustum::VF_RIGHT_PLANE], p);
187: 
188:             return p;
189:         }
190: 
191:         inline core::vector3df SViewFrustum::getFarRightDown() const
192:         {
193:             core::vector3df p;
194: 
195:             planes[scene::SViewFrustum::VF_FAR_PLANE].getIntersectionWithPlanes(
196:                 planes[scene::SViewFrustum::VF_BOTTOM_PLANE],
197:                 planes[scene::SViewFrustum::VF_RIGHT_PLANE], p);
198: 
199:             return p;
200:         }
201: 
202:         inline core::vector3df SViewFrustum::getNearLeftUp() const
203:         {
204:             core::vector3df p;
205: 
206:             planes[scene::SViewFrustum::VF_NEAR_PLANE].getIntersectionWithPlanes(
207:                 planes[scene::SViewFrustum::VF_TOP_PLANE],
208:                 planes[scene::SViewFrustum::VF_LEFT_PLANE], p);
209: 
210:             return p;
211:         }
212: 
213:         inline core::vector3df SViewFrustum::getNearLeftDown() const
214:         {
215:             core::vector3df p;
216: 
217:             planes[scene::SViewFrustum::VF_NEAR_PLANE].getIntersectionWithPlanes(
218:                 planes[scene::SViewFrustum::VF_BOTTOM_PLANE],
219:                 planes[scene::SViewFrustum::VF_LEFT_PLANE], p);
220: 
221:             return p;
222:         }
223: 
224:         inline core::vector3df SViewFrustum::getNearRightUp() const
225:         {
226:             core::vector3df p;
227: 
228:             planes[scene::SViewFrustum::VF_NEAR_PLANE].getIntersectionWithPlanes(
229:                 planes[scene::SViewFrustum::VF_TOP_PLANE],
230:                 planes[scene::SViewFrustum::VF_RIGHT_PLANE], p);
231: 
232:             return p;
233:         }
234: 
235:         inline core::vector3df SViewFrustum::getNearRightDown() const
236:         {
237:             core::vector3df p;
238: 
239:             planes[scene::SViewFrustum::VF_NEAR_PLANE].getIntersectionWithPlanes(
240:                 planes[scene::SViewFrustum::VF_BOTTOM_PLANE],
241:                 planes[scene::SViewFrustum::VF_RIGHT_PLANE], p);
242: 
243:             return p;
244:         }
245: 
246:         inline const core::aabbox3d<f32>&SViewFrustum::getBoundingBox() const
247:         {
248:             return boundingBox;
249:         }
250: 
251:         inline void SViewFrustum::recalculateBoundingBox()
252:         {
253:             boundingBox.reset (cameraPosition);
254: 
255:             boundingBox.addInternalPoint(getFarLeftUp());
256:             boundingBox.addInternalPoint(getFarRightUp());
257:             boundingBox.addInternalPoint(getFarLeftDown());
258:             boundingBox.addInternalPoint(getFarRightDown());
259:         }
260: 
261:         //! This constructor creates a view frustum based on a projection
262:         //! and/or view matrix.
263:         inline void SViewFrustum::setFrom(const core::matrix4 &mat)
264:         {
265:             // left clipping plane
266:             planes[VF_LEFT_PLANE].Normal.X = mat[3] + mat[0];
267:             planes[VF_LEFT_PLANE].Normal.Y = mat[7] + mat[4];
268:             planes[VF_LEFT_PLANE].Normal.Z = mat[11] + mat[8];
269:             planes[VF_LEFT_PLANE].D        = mat[15] + mat[12];
270: 
271:             // right clipping plane
272:             planes[VF_RIGHT_PLANE].Normal.X = mat[3] - mat[0];
273:             planes[VF_RIGHT_PLANE].Normal.Y = mat[7] - mat[4];
274:             planes[VF_RIGHT_PLANE].Normal.Z = mat[11] - mat[8];
275:             planes[VF_RIGHT_PLANE].D        = mat[15] - mat[12];
276: 
277:             // top clipping plane
278:             planes[VF_TOP_PLANE].Normal.X = mat[3] - mat[1];
279:             planes[VF_TOP_PLANE].Normal.Y = mat[7] - mat[5];
280:             planes[VF_TOP_PLANE].Normal.Z = mat[11] - mat[9];
281:             planes[VF_TOP_PLANE].D        = mat[15] - mat[13];
282: 
283:             // bottom clipping plane
284:             planes[VF_BOTTOM_PLANE].Normal.X = mat[3] + mat[1];
285:             planes[VF_BOTTOM_PLANE].Normal.Y = mat[7] + mat[5];
286:             planes[VF_BOTTOM_PLANE].Normal.Z = mat[11] + mat[9];
287:             planes[VF_BOTTOM_PLANE].D        = mat[15] + mat[13];
288: 
289:             // far clipping plane
290:             planes[VF_FAR_PLANE].Normal.X = mat[3] - mat[2];
291:             planes[VF_FAR_PLANE].Normal.Y = mat[7] - mat[6];
292:             planes[VF_FAR_PLANE].Normal.Z = mat[11] - mat[10];
293:             planes[VF_FAR_PLANE].D        = mat[15] - mat[14];
294: 
295:             // near clipping plane
296:             planes[VF_NEAR_PLANE].Normal.X = mat[2];
297:             planes[VF_NEAR_PLANE].Normal.Y = mat[6];
298:             planes[VF_NEAR_PLANE].Normal.Z = mat[10];
299:             planes[VF_NEAR_PLANE].D        = mat[14];
300: 
301:             // normalize normals
302:             u32 i;
303: 
304:             for (i = 0; i != VF_PLANE_COUNT; ++i)
305:             {
306:                 const f32 len = -core::reciprocal_squareroot(
307:                     planes[i].Normal.getLengthSQ());
308:                 planes[i].Normal *= len;
309:                 planes[i].D      *= len;
310:             }
311: 
312:             // make bounding box
313:             recalculateBoundingBox();
314:         }
315: 
316:         /*!
317:          *  View Frustum depends on Projection & View Matrix
318:          */
319:         inline core::matrix4&SViewFrustum::getTransform(video::E_TRANSFORMATION_STATE state)
320:         {
321:             u32 index = 0;
322: 
323:             switch (state)
324:             {
325:                 case video::ETS_PROJECTION:
326:                     index = SViewFrustum::ETS_PROJECTION; break;
327: 
328:                 case video::ETS_VIEW:
329:                     index = SViewFrustum::ETS_VIEW; break;
330: 
331:                 default:
332:                     break;
333:             }
334: 
335:             return Matrices[index];
336:         }
337: 
338:         /*!
339:          *  View Frustum depends on Projection & View Matrix
340:          */
341:         inline const core::matrix4&SViewFrustum::getTransform(video::E_TRANSFORMATION_STATE state) const
342:         {
343:             u32 index = 0;
344: 
345:             switch (state)
346:             {
347:                 case video::ETS_PROJECTION:
348:                     index = SViewFrustum::ETS_PROJECTION; break;
349: 
350:                 case video::ETS_VIEW:
351:                     index = SViewFrustum::ETS_VIEW; break;
352: 
353:                 default:
354:                     break;
355:             }
356: 
357:             return Matrices[index];
358:         }
359: 
360:         //! Clips a line to the frustum
361:         inline bool SViewFrustum::clipLine(core::line3d<f32> &line) const
362:         {
363:             bool wasClipped = false;
364: 
365:             for (u32 i = 0; i < VF_PLANE_COUNT; ++i)
366:             {
367:                 if (planes[i].classifyPointRelation(line.start) == core::ISREL3D_FRONT)
368:                 {
369:                     line.start = line.start.getInterpolated(line.end,
370:                             planes[i].getKnownIntersectionWithLine(line.start, line.end));
371:                     wasClipped = true;
372:                 }
373: 
374:                 if (planes[i].classifyPointRelation(line.end) == core::ISREL3D_FRONT)
375:                 {
376:                     line.end = line.start.getInterpolated(line.end,
377:                             planes[i].getKnownIntersectionWithLine(line.start, line.end));
378:                     wasClipped = true;
379:                 }
380:             }
381: 
382:             return wasClipped;
383:         }
384:     } // end namespace scene
385: } // end namespace irr
386: #endif

(End of file - total 386 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 54.3s)

_Thinking:_

Now I have a comprehensive picture. Let me summarize the analysis:

1. In main.cpp (07.Collision):
   - Camera is created with `smgr->addCameraSceneNodeFPS(...)` 
   - Position set via `camera->setPosition(core::vector3df(50, 50, -60))`
   - Target set via `camera->setTarget(core::vector3df(-70, 30, -60))`
   - Collision response animator is added for collision detection

2. In CCameraSceneNode.cpp:
   - `getViewMatrix()` returns `ViewArea.getTransform(video::ETS_VIEW)`
   - `cameraPosition` is stored in `SViewFrustum` structure

3. The view matrix calculation happens in `CCameraSceneNode::render()` (line 243-272):
   - Gets absolute position via `getAbsolutePosition()`
   - Calculates target direction: `tgtv = Target - pos`
   - Normalizes the direction
   - Builds camera look-at matrix: `buildCameraLookAtMatrixLH(pos, Target, up)`
   - Multiplies by affector if set
   - Calls `recalculateViewArea()` to update the frustum

4. In `recalculateViewArea()` (line 289-297):
   - Sets `ViewArea.cameraPosition = getAbsolutePosition()`
   - Creates combined projection*view matrix and calls `setFrom()` to extract frustum planes

Let me provide a clear summary to the user.




## CameraPosition 和 ViewMatrix 计算流程分析

### 1. Camera 设置 (main.cpp:151-154)

```cpp
scene::ICameraSceneNode *camera = smgr->addCameraSceneNodeFPS(0, 100.0f, .3f, ...);
camera->setPosition(core::vector3df(50, 50, -60));  // 位置
camera->setTarget(core::vector3df(-70, 30, -60));    // 看向目标
```

### 2. ViewMatrix 计算 (CCameraSceneNode.cpp:243-264)

在 `CCameraSceneNode::render()` 中每次渲染时更新:

```cpp
void CCameraSceneNode::render()
{
    core::vector3df pos  = getAbsolutePosition();        // 获取世界坐标
    core::vector3df tgtv = Target - pos;                  // 计算视线方向
    tgtv.normalize();
    
    // 构建左手坐标系相机视图矩阵
    ViewArea.getTransform(video::ETS_VIEW).buildCameraLookAtMatrixLH(pos, Target, up);
    ViewArea.getTransform(video::ETS_VIEW) *= Affector;  // 可选的视图矩阵修正器
    
    recalculateViewArea();
}
```

### 3. CameraPosition 更新 (CCameraSceneNode.cpp:289-297)

```cpp
void CCameraSceneNode::recalculateViewArea()
{
    ViewArea.cameraPosition = getAbsolutePosition();      // 相机位置
    
    // 结合投影和视图矩阵计算视锥体
    core::matrix4 m;
    m.setbyproduct_nocheck(ViewArea.getTransform(video::ETS_PROJECTION),
                          ViewArea.getTransform(video::ETS_VIEW));
    ViewArea.setFrom(m);                                  // 从矩阵提取6个裁剪平面
}
```

### 4. 获取方式

| 变量 | 获取方法 |
|------|----------|
| `cameraPosition` | `camera->getViewFrustum()->cameraPosition` 或 `getAbsolutePosition()` |
| `viewMatrix` | `camera->getViewMatrix()` |

### 流程图

```
setPosition() / setTarget()
        ↓
   render() 调用时
        ↓
getAbsolutePosition() → pos, tgtv
        ↓
buildCameraLookAtMatrixLH(pos, Target, up) → ViewMatrix
        ↓
recalculateViewArea() → 更新 cameraPosition 和 frustum planes
```

---

