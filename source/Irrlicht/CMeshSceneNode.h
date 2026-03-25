// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_MESH_SCENE_NODE_H_INCLUDED__
#define __C_MESH_SCENE_NODE_H_INCLUDED__

#include "IMeshSceneNode.h"
#include "IMesh.h"

namespace irr
{
namespace scene
{

/**
 * @brief Mesh scene node for rendering 3D meshes
 * 
 * Scene node that displays a mesh with configurable materials
 * and optional shadow volume.
 */
class CMeshSceneNode : public IMeshSceneNode
{
public:

    /**
     * @brief Constructor
     * @param mesh Mesh to display
     * @param parent Parent scene node
     * @param mgr Scene manager
     * @param id Node ID
     * @param position Position
     * @param rotation Rotation
     * @param scale Scale
     */
    CMeshSceneNode(IMesh *mesh, ISceneNode *parent, ISceneManager *mgr,    s32 id,
                   const core::vector3df &position = core::vector3df(0, 0, 0),
                   const core::vector3df &rotation = core::vector3df(0, 0, 0),
                   const core::vector3df &scale = core::vector3df(1.0f, 1.0f, 1.0f));

    /**
     * @brief Destructor
     */
    virtual ~CMeshSceneNode();

    /**
     * @brief Called when node is registered to scene
     */
    virtual void OnRegisterSceneNode();

    /**
     * @brief Render the mesh
     */
    virtual void render();

    /**
     * @brief Get bounding box
     * @return Axis-aligned bounding box
     */
    virtual const core::aabbox3d<f32>&getBoundingBox() const;

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
        return ESNT_MESH;
    }

    /**
     * @brief Set mesh
     * @param mesh New mesh to display
     */
    virtual void setMesh(IMesh *mesh);

    /**
     * @brief Get current mesh
     * @return Current mesh
     */
    virtual IMesh* getMesh(void)
    {
        return Mesh;
    }

    /**
     * @brief Add shadow volume
     * @param shadowMesh Mesh for shadow volume
     * @param id Shadow node ID
     * @param zfailmethod Use zfail method
     * @param infinity Shadow infinity distance
     * @return Shadow volume scene node
     */
    virtual IShadowVolumeSceneNode* addShadowVolumeSceneNode(const IMesh *shadowMesh,
                                                             s32 id, bool zfailmethod = true, f32 infinity = 10000.0f);

    /**
     * @brief Set read-only materials mode
     * @param readonly true for read-only materials
     */
    virtual void setReadOnlyMaterials(bool readonly);

    /**
     * @brief Check if materials are read-only
     * @return true if materials are read-only
     */
    virtual bool isReadOnlyMaterials() const;

    /**
     * @brief Clone this node and children
     * @param newParent New parent node
     * @param newManager New scene manager
     * @return Cloned scene node
     */
    virtual ISceneNode* clone(ISceneNode *newParent = 0, ISceneManager *newManager = 0);

    /**
     * @brief Remove child node
     * @param child Child to remove
     * @return true if child was removed
     */
    virtual bool removeChild(ISceneNode *child);

protected:

    /**
     * @brief Copy materials from mesh
     */
    void copyMaterials();

    core::array<video::SMaterial> Materials;    ///< Material array
    core::aabbox3d<f32>           Box;          ///< Bounding box
    video::SMaterial              ReadOnlyMaterial; ///< Read-only material

    IMesh                  *Mesh;               ///< Mesh to render
    IShadowVolumeSceneNode *Shadow;            ///< Shadow volume

    s32  PassCount;          ///< Render pass count
    bool ReadOnlyMaterials;  ///< Read-only materials flag
};
}   // end namespace scene
} // end namespace irr
#endif