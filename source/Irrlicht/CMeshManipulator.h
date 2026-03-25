// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_MESH_MANIPULATOR_H_INCLUDED__
#define __C_MESH_MANIPULATOR_H_INCLUDED__

#include "IMeshManipulator.h"

namespace irr
{
namespace scene
{

/**
 * @brief Mesh manipulation utilities
 * 
 * Provides easy mesh manipulation operations including:
 * - Normal recalculation
 * - Texture coordinate mapping
 * - Tangent space calculation
 * - Vertex optimization
 * - Mesh copying and conversion
 * 
 * @note Intended for fixing imported meshes, not runtime animation
 */
class CMeshManipulator : public IMeshManipulator
{
public:

    /**
     * @brief Flip surface normals
     * @param mesh Mesh to modify
     */
    virtual void flipSurfaces(scene::IMesh *mesh) const;

    /**
     * @brief Recalculate mesh normals
     * @param mesh Mesh to modify
     * @param smooth Use smooth normals
     * @param angleWeighted Angle-weighted normals
     */
    virtual void recalculateNormals(scene::IMesh *mesh, bool smooth = false, bool angleWeighted = false) const;

    /**
     * @brief Recalculate mesh buffer normals
     * @param buffer Mesh buffer to modify
     * @param smooth Use smooth normals
     * @param angleWeighted Angle-weighted normals
     */
    virtual void recalculateNormals(IMeshBuffer *buffer, bool smooth = false, bool angleWeighted = false) const;

    /**
     * @brief Create modifiable mesh copy
     * @param mesh Source mesh
     * @return New SMesh copy
     */
    virtual SMesh* createMeshCopy(scene::IMesh *mesh) const;

    /**
     * @brief Create planar UV mapping
     * @param mesh Mesh to modify
     * @param resolution Texture resolution
     */
    virtual void makePlanarTextureMapping(scene::IMesh *mesh, f32 resolution = 0.001f) const;

    /**
     * @brief Create planar UV mapping for buffer
     * @param meshbuffer Mesh buffer to modify
     * @param resolution Texture resolution
     */
    virtual void makePlanarTextureMapping(scene::IMeshBuffer *meshbuffer, f32 resolution = 0.001f) const;

    /**
     * @brief Create planar UV mapping with custom parameters
     * @param buffer Mesh buffer
     * @param resolutionS S resolution
     * @param resolutionT T resolution
     * @param axis Axis (0=X, 1=Y, 2=Z)
     * @param offset UV offset
     */
    void makePlanarTextureMapping(scene::IMeshBuffer *buffer, f32 resolutionS, f32 resolutionT, u8 axis, const core::vector3df &offset) const;

    /**
     * @brief Create planar UV mapping for mesh
     * @param mesh Mesh
     * @param resolutionS S resolution
     * @param resolutionT T resolution
     * @param axis Axis
     * @param offset UV offset
     */
    void makePlanarTextureMapping(scene::IMesh *mesh, f32 resolutionS, f32 resolutionT, u8 axis, const core::vector3df &offset) const;

    /**
     * @brief Recalculate tangent vectors
     * @param buffer Mesh buffer (must be tangent type)
     * @param recalculateNormals Recalculate normals too
     * @param smooth Use smooth normals
     * @param angleWeighted Angle-weighted
     */
    virtual void recalculateTangents(IMeshBuffer *buffer, bool recalculateNormals = false, bool smooth = false, bool angleWeighted = false) const;

    /**
     * @brief Recalculate tangents for mesh
     * @param mesh Mesh
     * @param recalculateNormals Recalculate normals too
     * @param smooth Use smooth normals
     * @param angleWeighted Angle-weighted
     */
    virtual void recalculateTangents(IMesh *mesh, bool recalculateNormals = false, bool smooth = false, bool angleWeighted = false) const;

    /**
     * @brief Create mesh with tangent vectors
     * @param mesh Source mesh
     * @param recalculateNormals Recalculate normals
     * @param smooth Smooth normals
     * @param angleWeighted Angle-weighted
     * @param recalculateTangents Calculate tangents
     * @return New mesh with tangents
     */
    virtual IMesh* createMeshWithTangents(IMesh *mesh, bool recalculateNormals = false, bool smooth = false, bool angleWeighted = false, bool recalculateTangents = true) const;

    /**
     * @brief Create mesh with 2 texture coordinates
     * @param mesh Source mesh
     * @return New mesh with 2 UV sets
     */
    virtual IMesh* createMeshWith2TCoords(IMesh *mesh) const;

    /**
     * @brief Create mesh with 1 texture coordinate
     * @param mesh Source mesh
     * @return New mesh with 1 UV set
     */
    virtual IMesh* createMeshWith1TCoords(IMesh *mesh) const;

    /**
     * @brief Create mesh with unique primitives
     * @param mesh Source mesh
     * @return New mesh with unique triangles (no shared vertices)
     */
    virtual IMesh* createMeshUniquePrimitives(IMesh *mesh) const;

    /**
     * @brief Create welded mesh
     * @param mesh Source mesh
     * @param tolerance Welding tolerance
     * @return New mesh with duplicated vertices removed
     */
    virtual IMesh* createMeshWelded(IMesh *mesh, f32 tolerance = core::ROUNDING_ERROR_f32) const;

    /**
     * @brief Get polygon count
     * @param mesh Source mesh
     * @return Number of polygons
     */
    virtual s32 getPolyCount(scene::IMesh *mesh) const;

    /**
     * @brief Get polygon count from animated mesh
     * @param mesh Source animated mesh
     * @return Number of polygons
     */
    virtual s32 getPolyCount(scene::IAnimatedMesh *mesh) const;

    /**
     * @brief Create animated mesh from static mesh
     * @param mesh Source mesh
     * @param type Animation type
     * @return New animated mesh
     */
    virtual IAnimatedMesh* createAnimatedMesh(scene::IMesh *mesh, scene::E_ANIMATED_MESH_TYPE type) const;

    /**
     * @brief Optimize mesh for vertex cache
     * @param mesh Source mesh
     * @return Optimized mesh
     */
    virtual IMesh* createForsythOptimizedMesh(const scene::IMesh *mesh) const;
};
}   // end namespace scene
} // end namespace irr
#endif