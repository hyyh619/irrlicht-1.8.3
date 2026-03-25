// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_CAMERA_SCENE_NODE_H_INCLUDED__
#define __C_CAMERA_SCENE_NODE_H_INCLUDED__

#include "ICameraSceneNode.h"
#include "SViewFrustum.h"

namespace irr
{
namespace scene
{

/**
 * @brief Camera scene node for 3D rendering
 * 
 * Provides camera functionality with configurable projection and view matrices,
 * near/far clipping planes, FOV, and input handling.
 */
class CCameraSceneNode : public ICameraSceneNode
{
public:

    /**
     * @brief Constructor
     * @param parent Parent scene node
     * @param mgr Scene manager
     * @param id Node ID
     * @param position Initial camera position
     * @param lookat Initial look-at target
     */
    CCameraSceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,
                     const core::vector3df &position = core::vector3df(0, 0, 0),
                     const core::vector3df &lookat = core::vector3df(0, 0, 100));

    /**
     * @brief Set projection matrix
     * @param projection New projection matrix
     * @param isOrthogonal true for orthogonal projection
     * @note Matrix persists until setNearValue, setFarValue, setAspectRatio, or setFOV is called
     */
    virtual void setProjectionMatrix(const core::matrix4 &projection, bool isOrthogonal = false);

    /**
     * @brief Get current projection matrix
     * @return Current projection matrix
     */
    virtual const core::matrix4&getProjectionMatrix() const;

    /**
     * @brief Get current view matrix
     * @return Current view matrix
     */
    virtual const core::matrix4&getViewMatrix() const;

    /**
     * @brief Set view matrix affector
     * @param affector Affector matrix to apply to view matrix
     */
    virtual void setViewMatrixAffector(const core::matrix4 &affector);

    /**
     * @brief Get view matrix affector
     * @return Current view matrix affector
     */
    virtual const core::matrix4&getViewMatrixAffector() const;

    /**
     * @brief Handle input events
     * @param event Input event to handle
     * @return true if event was consumed
     */
    virtual bool OnEvent(const SEvent &event);

    /**
     * @brief Set look-at target
     * @param pos Target position
     * @note May also change rotation if bound via bindTargetAndRotation()
     */
    virtual void setTarget(const core::vector3df &pos);

    /**
     * @brief Set camera rotation
     * @param rotation Rotation in degrees
     * @note May also change target if bound via bindTargetAndRotation()
     */
    virtual void setRotation(const core::vector3df &rotation);

    /**
     * @brief Get look-at target
     * @return Current target position
     */
    virtual const core::vector3df&getTarget() const;

    /**
     * @brief Set up vector
     * @param pos Up vector direction
     */
    virtual void setUpVector(const core::vector3df &pos);

    /**
     * @brief Get up vector
     * @return Current up vector
     */
    virtual const core::vector3df&getUpVector() const;

    /**
     * @brief Get near clipping plane distance
     * @return Near plane distance
     */
    virtual f32 getNearValue() const;

    /**
     * @brief Get far clipping plane distance
     * @return Far plane distance
     */
    virtual f32 getFarValue() const;

    /**
     * @brief Get aspect ratio
     * @return Current aspect ratio
     */
    virtual f32 getAspectRatio() const;

    /**
     * @brief Get field of view
     * @return FOV in radians
     */
    virtual f32 getFOV() const;

    /**
     * @brief Set near clipping plane
     * @param zn Near plane distance (default: 1.0f)
     */
    virtual void setNearValue(f32 zn);

    /**
     * @brief Set far clipping plane
     * @param zf Far plane distance (default: 2000.0f)
     */
    virtual void setFarValue(f32 zf);

    /**
     * @brief Set aspect ratio
     * @param aspect Aspect ratio (default: 4.0f/3.0f)
     */
    virtual void setAspectRatio(f32 aspect);

    /**
     * @brief Set field of view
     * @param fovy FOV in radians (default: PI/3.5f)
     */
    virtual void setFOV(f32 fovy);

    /**
     * @brief Called when node is registered to scene
     */
    virtual void OnRegisterSceneNode();

    /**
     * @brief Render the camera
     */
    virtual void render();

    /**
     * @brief Get bounding box
     * @return Axis-aligned bounding box
     */
    virtual const core::aabbox3d<f32>&getBoundingBox() const;

    /**
     * @brief Get view frustum
     * @return Pointer to view frustum
     */
    virtual const SViewFrustum* getViewFrustum() const;

    /**
     * @brief Enable/disable input handling
     * @param enabled true to enable input handling
     */
    virtual void setInputReceiverEnabled(bool enabled);

    /**
     * @brief Check if input handling is enabled
     * @return true if input handling is enabled
     */
    virtual bool isInputReceiverEnabled() const;

    /**
     * @brief Serialize node attributes
     * @param out Output attributes
     * @param options Read/write options
     */
    virtual void serializeAttributes(io::IAttributes *out, io::SAttributeReadWriteOptions *options = 0) const;

    /**
     * @brief Deserialize node attributes
     * @param in Input attributes
     * @param options Read/write options
     */
    virtual void deserializeAttributes(io::IAttributes *in, io::SAttributeReadWriteOptions *options = 0);

    /**
     * @brief Get node type
     * @return Scene node type identifier
     */
    virtual ESCENE_NODE_TYPE getType() const
    {
        return ESNT_CAMERA;
    }

    /**
     * @brief Bind target and rotation together
     * @param bound true to bind, false to unbind
     */
    virtual void bindTargetAndRotation(bool bound);

    /**
     * @brief Check if target and rotation are bound
     * @return true if bound together
     */
    virtual bool getTargetAndRotationBinding(void) const;

    /**
     * @brief Clone this camera and children
     * @param newParent New parent node
     * @param newManager New scene manager
     * @return Cloned scene node
     */
    virtual ISceneNode* clone(ISceneNode *newParent = 0, ISceneManager *newManager = 0);

protected:

    /**
     * @brief Recalculate projection matrix
     */
    void recalculateProjectionMatrix();
    
    /**
     * @brief Recalculate view frustum
     */
    void recalculateViewArea();

    core::vector3df Target;      ///< Look-at target
    core::vector3df UpVector;    ///< Up vector

    f32 Fovy;     ///< Field of view in radians
    f32 Aspect;    ///< Aspect ratio
    f32 ZNear;     ///< Near clipping plane
    f32 ZFar;      ///< Far clipping plane

    SViewFrustum  ViewArea;  ///< View frustum
    core::matrix4 Affector;  ///< View matrix affector

    bool InputReceiverEnabled;       ///< Input handling enabled
    bool TargetAndRotationAreBound;   ///< Target and rotation bound
};
}   // end namespace
} // end namespace
#endif