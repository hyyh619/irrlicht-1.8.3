// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_MD2_MESH_FILE_LOADER_H_INCLUDED__
#define __C_MD2_MESH_FILE_LOADER_H_INCLUDED__

#include "IMeshLoader.h"

namespace irr
{
namespace scene
{
class CAnimatedMeshMD2;

//! Meshloader capable of loading MD2 files
class CMD2MeshFileLoader : public IMeshLoader
{
public:

    /**
     * @brief Constructor
     */
    CMD2MeshFileLoader();

    /**
     * @brief Check if the file can be loaded based on extension
     * @param filename The file name to check
     * @return true if the file extension is loadable by this loader
     */
    virtual bool isALoadableFileExtension(const io::path &filename) const;

    /**
     * @brief Create an animated mesh from the MD2 file
     * @param file Pointer to the file to load
     * @return Pointer to the created animated mesh, or 0 if loading failed
     * @note The returned mesh must be dropped by the caller when no longer needed
     * @see IAnimatedMesh::drop()
     */
    virtual IAnimatedMesh* createMesh(io::IReadFile *file);

private:
    /**
     * @brief Load file data into the MD2 mesh
     * @param file Pointer to the file to load
     * @param mesh Pointer to the target mesh
     * @return true if loading succeeded
     */
    bool loadFile(io::IReadFile *file, CAnimatedMeshMD2 *mesh);
};
}   // end namespace scene
} // end namespace irr
#endif // __C_MD2_MESH_LOADER_H_INCLUDED__