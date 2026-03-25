// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

// New skinned mesh

#ifndef __C_SKINNED_MESH_H_INCLUDED__
#define __C_SKINNED_MESH_H_INCLUDED__

#include "ISkinnedMesh.h"
#include "SMeshBuffer.h"
#include "S3DVertex.h"
#include "irrString.h"
#include "matrix4.h"
#include "quaternion.h"

namespace irr
{
namespace scene
{
class IAnimatedMeshSceneNode;
class IBoneSceneNode;

/**
 * @brief Skinned mesh with skeletal animation
 * 
 * Supports skeletal animation with:
 * - Bone/joint hierarchy
 * - Position, rotation, scale keyframe animation
 * - Vertex skinning with weights
 * - Software and hardware skinning
 * - Animation blending
 */
class CSkinnedMesh : public ISkinnedMesh
{
public:

    /**
     * @brief Constructor
     */
    CSkinnedMesh();

    /**
     * @brief Destructor
     */
    virtual ~CSkinnedMesh();

    /**
     * @brief Get frame count
     * @return Number of animation frames (1 = static mesh)
     */
    virtual u32 getFrameCount() const;

    /**
     * @brief Get animation speed
     * @return Frames per second
     */
    virtual f32 getAnimationSpeed() const;

    /**
     * @brief Set animation speed
     * @param fps Frames per second
     */
    virtual void setAnimationSpeed(f32 fps);

    /**
     * @brief Get mesh for frame
     * @param frame Animation frame
     * @param detailLevel Detail level (ignored)
     * @param startFrameLoop Start frame loop
     * @param endFrameLoop End frame loop
     * @return Renderable mesh
     */
    virtual IMesh* getMesh(s32 frame, s32 detailLevel = 255, s32 startFrameLoop = -1, s32 endFrameLoop = -1);

    /**
     * @brief Animate mesh joints
     * @param frame Animation frame
     * @param blend Blend factor (0=old, 1=new)
     */
    virtual void animateMesh(f32 frame, f32 blend);

    /**
     * @brief Perform software skinning
     */
    virtual void skinMesh();

    /**
     * @brief Get mesh buffer count
     * @return Number of mesh buffers
     */
    virtual u32 getMeshBufferCount() const;

    /**
     * @brief Get mesh buffer by index
     * @param nr Buffer index
     * @return Mesh buffer pointer
     */
    virtual IMeshBuffer* getMeshBuffer(u32 nr) const;

    /**
     * @brief Get mesh buffer by material
     * @param material Material to search for
     * @return Mesh buffer with matching material
     */
    virtual IMeshBuffer* getMeshBuffer(const video::SMaterial &material) const;

    /**
     * @brief Get bounding box
     * @return Axis-aligned bounding box
     */
    virtual const core::aabbox3d<f32>&getBoundingBox() const;

    /**
     * @brief Set bounding box
     * @param box New bounding box
     */
    virtual void setBoundingBox(const core::aabbox3df &box);

    /**
     * @brief Set material flag for all buffers
     * @param flag Material flag
     * @param newvalue Flag value
     */
    virtual void setMaterialFlag(video::E_MATERIAL_FLAG flag, bool newvalue);

    /**
     * @brief Set hardware mapping hint
     * @param newMappingHint Mapping hint
     * @param buffer Buffer type
     */
    virtual void setHardwareMappingHint(E_HARDWARE_MAPPING newMappingHint, E_BUFFER_TYPE buffer = EBT_VERTEX_AND_INDEX);

    /**
     * @brief Mark mesh as dirty
     * @param buffer Buffer type to reload
     */
    virtual void setDirty(E_BUFFER_TYPE buffer = EBT_VERTEX_AND_INDEX);

    /**
     * @brief Get mesh type
     * @return Animated mesh type
     */
    virtual E_ANIMATED_MESH_TYPE getMeshType() const;

    /**
     * @brief Get joint count
     * @return Number of bones/joints
     */
    virtual u32 getJointCount() const;

    /**
     * @brief Get joint name
     * @param number Joint index
     * @return Joint name
     */
    virtual const c8* getJointName(u32 number) const;

    /**
     * @brief Get joint index by name
     * @param name Joint name
     * @return Joint index, -1 if not found
     */
    virtual s32 getJointNumber(const c8 *name) const;

    /**
     * @brief Copy animation from another mesh
     * @param mesh Source skinned mesh
     * @return true if successful
     */
    virtual bool useAnimationFrom(const ISkinnedMesh *mesh);

    /**
     * @brief Set normal update during animation
     * @param on Enable/disable normal updating
     */
    virtual void updateNormalsWhenAnimating(bool on);

    /**
     * @brief Set interpolation mode
     * @param mode Interpolation mode
     */
    virtual void setInterpolationMode(E_INTERPOLATION_MODE mode);

    /**
     * @brief Convert mesh to tangent format
     */
    virtual void convertMeshToTangents();

    /**
     * @brief Check if mesh is static
     * @return true if no animation
     */
    virtual bool isStatic();

    /**
     * @brief Enable/disable hardware skinning
     * @param on Enable flag
     * @return true if successful
     */
    virtual bool setHardwareSkinning(bool on);

    // --- Loader interface ---

    /**
     * @brief Get mesh buffers (for loaders)
     * @return Array of skin mesh buffers
     */
    virtual core::array<SSkinMeshBuffer*>&getMeshBuffers();

    /**
     * @brief Get all joints (for loaders)
     * @return Array of all joints
     */
    virtual core::array<SJoint*>&getAllJoints();

    /**
     * @brief Get all joints (const)
     * @return Array of all joints
     */
    virtual const core::array<SJoint*>&getAllJoints() const;

    /**
     * @brief Finalize mesh after loading
     */
    virtual void finalize();

    /**
     * @brief Add new mesh buffer
     * @return New skin mesh buffer
     */
    virtual SSkinMeshBuffer* addMeshBuffer();

    /**
     * @brief Add new joint
     * @param parent Parent joint (optional)
     * @return New joint
     */
    virtual SJoint* addJoint(SJoint *parent = 0);

    /**
     * @brief Add position keyframe
     * @param joint Joint to add key to
     * @return New position key
     */
    virtual SPositionKey* addPositionKey(SJoint *joint);
    
    /**
     * @brief Add rotation keyframe
     * @param joint Joint to add key to
     * @return New rotation key
     */
    virtual SRotationKey* addRotationKey(SJoint *joint);
    
    /**
     * @brief Add scale keyframe
     * @param joint Joint to add key to
     * @return New scale key
     */
    virtual SScaleKey* addScaleKey(SJoint *joint);

    /**
     * @brief Add vertex weight
     * @param joint Joint to weight
     * @return New weight
     */
    virtual SWeight* addWeight(SJoint *joint);

    /**
     * @brief Update bounding box
     */
    virtual void updateBoundingBox(void);

    /**
     * @brief Recover joints from mesh
     * @param jointChildSceneNodes Array to populate
     */
    void recoverJointsFromMesh(core::array<IBoneSceneNode*> &jointChildSceneNodes);

    /**
     * @brief Transfer joint data to mesh
     * @param jointChildSceneNodes Joint scene nodes
     */
    void transferJointsToMesh(const core::array<IBoneSceneNode*> &jointChildSceneNodes);

    /**
     * @brief Transfer joint hints to mesh
     * @param jointChildSceneNodes Joint scene nodes
     */
    void transferOnlyJointsHintsToMesh(const core::array<IBoneSceneNode*> &jointChildSceneNodes);

    /**
     * @brief Create joint scene nodes
     * @param jointChildSceneNodes Array to populate
     * @param node Parent scene node
     * @param smgr Scene manager
     */
    void addJoints(core::array<IBoneSceneNode*> &jointChildSceneNodes,
                   IAnimatedMeshSceneNode *node,
                   ISceneManager *smgr);

private:

    /**
     * @brief Check for animation data
     */
    void checkForAnimation();

    /**
     * @brief Normalize vertex weights
     */
    void normalizeWeights();

    /**
     * @brief Build local animated matrices
     */
    void buildAllLocalAnimatedMatrices();

    /**
     * @brief Build global animated matrices
     * @param Joint Current joint
     * @param ParentJoint Parent joint
     */
    void buildAllGlobalAnimatedMatrices(SJoint *Joint = 0, SJoint *ParentJoint = 0);

    /**
     * @brief Get frame data with hints
     * @param frame Animation frame
     * @param Node Joint node
     * @param position Output position
     * @param positionHint Position hint
     * @param scale Output scale
     * @param scaleHint Scale hint
     * @param rotation Output rotation
     * @param rotationHint Rotation hint
     */
    void getFrameData(f32 frame, SJoint *Node,
                      core::vector3df &position, s32 &positionHint,
                      core::vector3df &scale, s32 &scaleHint,
                      core::quaternion &rotation, s32 &rotationHint);

    /**
     * @brief Calculate global matrices
     * @param Joint Joint to calculate
     * @param ParentJoint Parent joint
     */
    void calculateGlobalMatrices(SJoint *Joint, SJoint *ParentJoint);

    /**
     * @brief Skin single joint
     * @param Joint Joint to skin
     * @param ParentJoint Parent joint
     */
    void skinJoint(SJoint *Joint, SJoint *ParentJoint);

    /**
     * @brief Calculate tangent vectors
     * @param normal Normal vector
     * @param tangent Tangent vector
     * @param binormal Binormal vector
     * @param vt1 Vertex 1
     * @param vt2 Vertex 2
     * @param vt3 Vertex 3
     * @param tc1 Tex coord 1
     * @param tc2 Tex coord 2
     * @param tc3 Tex coord 3
     */
    void calculateTangents(core::vector3df &normal,
                           core::vector3df &tangent, core::vector3df &binormal,
                           core::vector3df &vt1, core::vector3df &vt2, core::vector3df &vt3,
                           core::vector2df &tc1, core::vector2df &tc2, core::vector2df &tc3);

    core::array<SSkinMeshBuffer*> *SkinningBuffers;  ///< Skinning buffers

    core::array<SSkinMeshBuffer*> LocalBuffers;  ///< Local mesh buffers

    core::array<SJoint*> AllJoints;  ///< All joints in mesh
    core::array<SJoint*> RootJoints;  ///< Root joints

    core::array<core::array<bool> > Vertices_Moved;  ///< Vertex moved flags

    core::aabbox3d<f32> BoundingBox;  ///< Bounding box

    f32 AnimationFrames;  ///< Total animation frames
    f32 FramesPerSecond;  ///< Animation speed

    f32  LastAnimatedFrame;  ///< Last animated frame
    bool SkinnedLastFrame;  ///< Skinned flag

    E_INTERPOLATION_MODE InterpolationMode : 8;  ///< Interpolation mode

    bool HasAnimation;  ///< Has animation flag
    bool PreparedForSkinning;  ///< Prepared for skinning
    bool AnimateNormals;  ///< Animate normals
    bool HardwareSkinning;  ///< Hardware skinning enabled
};
}   // end namespace scene
} // end namespace irr
#endif