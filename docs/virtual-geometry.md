# Virtual Geometry（虚拟几何体）技术详解

## 一、概念定义

Virtual Geometry（虚拟几何体）是现代游戏引擎中的一项革命性渲染技术，其核心思想是**对几何体数据进行虚拟化处理**,使GPU能够在运行时动态决定渲染哪些三角形,而不是依赖传统的预计算LOD(Level of Detail)系统。这项技术最早由Epic Games在Unreal Engine 5的Nanite系统中引入,随后被其他引擎如Bevy借鉴采用。

Virtual Geometry的目标是解决传统渲染管线中的几个根本性困境:大量绘制调用(Draw Call)带来的CPU瓶颈、显存容量限制、以及高Poly模型无法实时渲染的问题。通过虚拟几何体技术,现代游戏引擎可以渲染包含数万亿三角形的场景,同时保持实时帧率。

## 二、核心技术组成

### 2.1 Meshlet(网格片段)

Meshlet是Virtual Geometry的基本处理单元,它是一个小型三角形簇(通常包含64到128个三角形),每个Meshlet共享一个局部包围球(Bounding Sphere)。这种设计使得GPU能够以Meshlet为单位进行精细的裁剪和LOD选择,而不是以整个模型为单位。

每个Meshlet包含以下数据:

- **顶点数据**:局部坐标系下的顶点位置
- **索引数据**:三角形索引
- **包围球**:用于视锥裁剪和LOD计算
- **Screen Space Error(SSE)值**:用于决定是否需要细分

### 2.2 两遍遮挡裁剪(Two-Pass Occlusion Culling)

Virtual Geometry采用两遍遮挡裁剪来减少过度绘制(Overdraw):

**第一遍**:基于上一帧渲染的深度信息,绘制那些上一帧实际贡献了像素的Cluster。这些Cluster是当前帧可见内容的良好近似。

**第二遍**:使用深度金字塔(Depth Pyramid)对第一遍未处理的Cluster进行裁剪。具体做法是将Cluster的包围盒与上一帧的深度图进行对比,消除被遮挡的Cluster。只有通过两遍裁剪的Cluster才会被提交渲染。

### 2.3 软件光栅化与硬件光栅化混合

Nanite的核心创新在于结合了软件光栅化和硬件光栅化:

- 对于**屏幕覆盖率高的三角形**,使用传统硬件光栅化,利用GPU的Early-Z等优化
- 对于**屏幕覆盖率低的微小三角形**,使用软件光栅化(在Compute Shader中实现),通过原子操作(atomicMax)写入Visibility Buffer

这种方法避免了小三角形带来的光栅化效率问题。

### 2.4 LOD选择机制

Virtual Geometry使用**Screen Space Error(SSE)**来自动决定LOD级别。每个Meshlet有一个父节点(简化版本)和多个子节点(精细版本)。当某个Meshlet的SSE值超过预设阈值时,系统自动切换到更精细的子Meshlet。

为了保证并行处理的一致性,每个Cluster独立评估自身的SSE值,做出相同的LOD决策。这种去中心化的设计使得GPU可以高效地处理海量几何体。

## 三、算法流程

### 3.1 预处理阶段

在资源加载时,引擎执行以下预处理:

```
1. Meshlet划分
   - 将模型分解为若干个64-128个三角形的小簇
   - 为每个簇计算包围球信息

2. 构建议程加速树
   - 从原始Meshlet开始,逐步简化
   - 形成分层树结构

3. 紧凑编码与压缩
   - 将Meshlet顶点量化压缩
   - 存储为位流格式以节省显存

4. 写入Visibility Buffer结构
   - 预分配用于运行时裁剪的数据结构
```

### 3.2 运行时阶段

每一帧的渲染流程如下:

```
Step 1: 视锥裁剪(Frustum Culling)
        - GPU并行测试所有Meshlet的包围球
        - 剔除完全在视锥体外的Meshlet

Step 2: 遮挡裁剪(Occlusion Culling)
        - 第一遍:绘制上一帧可见的Cluster
        - 第二遍:使用深度金字塔剔除被遮挡的Cluster

Step 3: LOD选择
        - 计算每个存活Meshlet的Screen Space Error
        - 根据阈值决定使用父级还是子级Meshlet

Step 4: 光栅化渲染
        - 根据三角形屏幕覆盖率选择硬件或软件光栅化
        - 将三角形写入Visibility Buffer

Step 5: 着色渲染
        - 按照常规流程对存活的片元进行着色处理
```

## 四、代表性实现

### 4.1 Nanite (Unreal Engine 5)

Nanite是Virtual Geometry的标志性实现,支持数万亿三角形场景的实时渲染。它采用了完全流式(Streaming)的数据管理,所有几何体数据都可以从磁盘流式加载,无需将整个场景放入显存。

Nanite的软件光栅化器能够在Compute Shader中处理微小三角形,这是其相对于传统光栅化器的关键优势。

### 4.2 Bevy Meshlets

Bevy引擎的Virtual Geometry实现(早期称为meshlets)采用较为简化的设计:

- 使用Depth Pyramid进行遮挡裁剪
- 使用Compute Shader进行软件光栅化
- 更侧重于教育目的,展示了Virtual Geometry的基本原理

### 4.3 Three.js WebGL实现

在WebGL环境中,开发者可以使用以下技术实现简化版的Virtual Geometry:

- 使用`InstancedMesh`或`BatchedMesh`配合Compute Culling
- 通过在数据纹理中编码Meshlet的包围球信息
- 在顶点着色器中进行视锥裁剪和LOD选择
- 可以获得接近原生实现的性能

## 五、技术优势与局限

### 优势

| 优势 | 说明 |
|------|------|
| **消除Draw Call瓶颈** | 使场景复杂度与Draw Call数量解耦 |
| **自动LOD选择** | 艺术家无需手动创建多个细节级别 |
| **显存利用率优化** | 按需加载几何体数据 |
| **支持海量几何体** | 为电影级画面质量提供技术基础 |

### 局限性

| 局限性 | 说明 |
|--------|------|
| 仅支持不透明静态网格 | 不支持蒙皮动画 |
| 需要额外预处理时间 | 磁盘空间增加 |
| 小三角形效率问题 | 需要软件光栅化补充 |
| 实现复杂度高 | 需要深入的GPU编程知识 |

## 六、总结

Virtual Geometry代表了游戏渲染技术的未来方向——将几何体处理的决策权从CPU转移到GPU。

通过以下核心技术:

1. **Meshlet抽象** - 将几何体分解为可并行处理的小簇
2. **两遍遮挡裁剪** - 利用历史帧数据进行高效裁剪
3. **软硬件混合光栅化** - 兼顾效率与精度
4. **自动LOD选择** - 基于SSE的智能化决定

这些技术使得实时渲染超大规模几何体成为可能。随着更多引擎采用类似技术,电影级画面质量和实时渲染之间的界限将进一步模糊。

## 参考资料

- [A Deep Dive into Nanite Virtualized Geometry - SIGGRAPH 2021](https://www.youtube.com/watch?v=eviSykqSUUw)
- [Nanite Virtualized Geometry - Epic Games Documentation](https://dev.epicgames.com/documentation/unreal-engine/nanite-virtualized-geometry-in-unreal-engine)
- [Virtual Geometry in Bevy 0.14 - JMS55](https://jms55.github.io/posts/2024-06-09-virtual-geometry-bevy-0-14/)
- [Virtual Geometry in Bevy 0.15](https://jms55.github.io/posts/2024-11-14-virtual-geometry-bevy-0-15/)
- [Beyond Polygons: Implementing Virtualized Geometry in Three.js](https://javascript-news.org/beyond-polygons-implementing-virtualized-geometry-and-micro-poly-rendering-in-three-js)