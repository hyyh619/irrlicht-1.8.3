// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_BILLBOARD_SCENE_NODE_H_INCLUDED__
#define __C_BILLBOARD_SCENE_NODE_H_INCLUDED__

#include "IBillboardSceneNode.h"
#include "S3DVertex.h"

namespace irr
{
namespace scene
{

/**
 * @brief Billboard scene node
 * 
 * A billboard is a 2D element that always faces the camera,
 * commonly used for particles, sprites, and effects.
 */
class CBillboardSceneNode : virtual public IBillboardSceneNode
{
public:

    /**
     * @brief Constructor
     * @param parent Parent scene node
     * @param mgr Scene manager
     * @param id Node ID
     * @param position Billboard position
     * @param size Billboard size
     * @param colorTop Top vertex color
     * @param colorBottom Bottom vertex color
     */
    CBillboardSceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,
                        const core::vector3df &position, const core::dimension2d<f32> &size,
                        video::SColor colorTop = video::SColor(0xFFFFFFFF),
                        video::SColor colorBottom = video::SColor(0xFFFFFFFF));

    /**
     * @brief Called when node is registered to scene
     */
    virtual void OnRegisterSceneNode();

    /**
     * @brief Render the billboard
     */
    virtual void render();

    /**
     * @brief Get bounding box
     * @return Axis-aligned bounding box
     */
    virtual const core::aabbox3d<f32>&getBoundingBox() const;

    /**
     * @brief Set billboard size
     * @param size New size
     */
    virtual void setSize(const core::dimension2d<f32> &size);

    /**
     * @brief Set billboard size with different edge widths
     * @param height Billboard height
     * @param bottomEdgeWidth Width of bottom edge
     * @param topEdgeWidth Width of top edge
     */
    virtual void setSize(f32 height, f32 bottomEdgeWidth, f32 topEdgeWidth);

    /**
     * @brief Get billboard size
     * @return Current size
     */
    virtual const core::dimension2d<f32>&getSize() const;

    /**
     * @brief Get billboard size with edge widths
     * @param height Height output
     * @param bottomEdgeWidth Bottom edge width output
     * @param topEdgeWidth Top edge width output
     */
    virtual void getSize(f32 &height, f32 &bottomEdgeWidth, f32 &topEdgeWidth) const;

    /**
     * @brief Get material by index
     * @param i Material index
     * @return Reference to material
     */
    virtual video::SMaterial&getMaterial(u32 i);

    /**
     * @brief Get material count
     * @return Number of materials
     */
    virtual u32 getMaterialCount() const;

    /**
     * @brief Set vertex color
     * @param overallColor Color for all vertices
     */
    virtual void setColor(const video::SColor &overallColor);

    /**
     * @brief Set vertex colors separately
     * @param topColor Color for top vertices
     * @param bottomColor Color for bottom vertices
     */
    virtual void setColor(const video::SColor &topColor,
                          const video::SColor &bottomColor);

    /**
     * @brief Get vertex colors
     * @param topColor Top vertex color output
     * @param bottomColor Bottom vertex color output
     */
    virtual void getColor(video::SColor &topColor,
                          video::SColor &bottomColor) const;

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
        return ESNT_BILLBOARD;
    }

    /**
     * @brief Clone this node and children
     * @param newParent New parent node
     * @param newManager New scene manager
     * @return Cloned scene node
     */
    virtual ISceneNode* clone(ISceneNode *newParent = 0, ISceneManager *newManager = 0);

private:

    core::dimension2d<f32> Size;          ///< Billboard size (width = bottom edge)
    f32                    TopEdgeWidth;  ///< Top edge width
    core::aabbox3d<f32>    BBox;          ///< Bounding box
    video::SMaterial       Material;      ///< Material

    video::S3DVertex vertices[4];  ///< Billboard vertices
    u16              indices[6];    ///< Triangle indices
};
}   // end namespace scene
} // end namespace irr
#endif