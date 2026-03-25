// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_LIGHT_SCENE_NODE_H_INCLUDED__
#define __C_LIGHT_SCENE_NODE_H_INCLUDED__

#include "ILightSceneNode.h"

namespace irr
{
namespace scene
{

/**
 * @brief Dynamic light scene node
 * 
 * Scene node representing a dynamic light source.
 * Can be switched on/off by making it visible or not.
 */
class CLightSceneNode : public ILightSceneNode
{
public:

    /**
     * @brief Constructor
     * @param parent Parent scene node
     * @param mgr Scene manager
     * @param id Node ID
     * @param position Light position
     * @param color Light color
     * @param range Light range
     */
    CLightSceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,
                    const core::vector3df &position, video::SColorf color, f32 range);

    virtual ~CLightSceneNode() { }

    /**
     * @brief Called when node is registered to scene
     */
    virtual void OnRegisterSceneNode();

    /**
     * @brief Render the light
     */
    virtual void render();

    /**
     * @brief Set light data
     * @param light Light parameters
     */
    virtual void setLightData(const video::SLight &light);

    /**
     * @brief Get light data (const)
     * @return Light parameters
     */
    virtual const video::SLight&getLightData() const;

    /**
     * @brief Get light data
     * @return Light parameters
     */
    virtual video::SLight&getLightData();

    /**
     * @brief Set visibility
     * @param isVisible Visibility state
     */
    virtual void setVisible(bool isVisible);

    /**
     * @brief Get bounding box
     * @return Axis-aligned bounding box
     */
    virtual const core::aabbox3d<f32>&getBoundingBox() const;

    /**
     * @brief Get node type
     * @return Scene node type identifier
     */
    virtual ESCENE_NODE_TYPE getType() const
    {
        return ESNT_LIGHT;
    }

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
     * @brief Clone this node and children
     * @param newParent New parent node
     * @param newManager New scene manager
     * @return Cloned scene node
     */
    virtual ISceneNode* clone(ISceneNode *newParent = 0, ISceneManager *newManager = 0);


    /**
     * @brief Set light radius
     * @param radius Light influence radius
     * @note Sets attenuation to (0, 1/radius, 0)
     */
    virtual void setRadius(f32 radius);

    /**
     * @brief Get light radius
     * @return Current radius
     */
    virtual f32 getRadius() const;

    /**
     * @brief Set light type
     * @param type Light type
     */
    virtual void setLightType(video::E_LIGHT_TYPE type);

    /**
     * @brief Get light type
     * @return Current light type
     */
    virtual video::E_LIGHT_TYPE getLightType() const;

    /**
     * @brief Enable shadow casting
     * @param shadow Enable/disable shadow casting
     */
    virtual void enableCastShadow(bool shadow = true);

    /**
     * @brief Check if shadow casting is enabled
     * @return true if shadows are enabled
     */
    virtual bool getCastShadow() const;
private:

    video::SLight       LightData;       ///< Light parameters
    core::aabbox3d<f32> BBox;           ///< Bounding box
    s32                 DriverLightIndex; ///< GPU light index
    bool                LightIsOn;       ///< Light on/off state
    
    /**
     * @brief Recalculate light
     */
    void doLightRecalc();
};
}   // end namespace scene
} // end namespace irr
#endif