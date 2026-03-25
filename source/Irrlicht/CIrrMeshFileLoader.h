// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IRR_MESH_FILE_LOADER_H_INCLUDED__
#define __C_IRR_MESH_FILE_LOADER_H_INCLUDED__

#include "IMeshLoader.h"
#include "IFileSystem.h"
#include "IVideoDriver.h"
#include "irrString.h"
#include "SMesh.h"
#include "SMeshBuffer.h"
#include "CDynamicMeshBuffer.h"
#include "ISceneManager.h"

namespace irr
{
namespace scene
{
//! Meshloader capable of loading .irrmesh meshes, the Irrlicht Engine mesh format for static meshes
class CIrrMeshFileLoader : public IMeshLoader
{
public:

    /**
     * @brief Constructor
     * @param smgr Pointer to the scene manager
     * @param fs Pointer to the file system
     */
    CIrrMeshFileLoader(scene::ISceneManager *smgr, io::IFileSystem *fs);

    /**
     * @brief Check if the file can be loaded based on extension
     * @param filename The file name to check
     * @return true if the file extension is loadable by this loader
     */
    virtual bool isALoadableFileExtension(const io::path &filename) const;

    /**
     * @brief Create an animated mesh from the IrrMesh file
     * @param file Pointer to the file to load
     * @return Pointer to the created animated mesh, or 0 if loading failed
     * @note The returned mesh must be dropped by the caller when no longer needed
     * @see IAnimatedMesh::drop()
     */
    virtual IAnimatedMesh* createMesh(io::IReadFile *file);

private:

    /**
     * @brief Read mesh section and create mesh
     * @param reader XML reader positioned at mesh element
     * @return Pointer to created animated mesh
     */
    IAnimatedMesh* readMesh(io::IXMLReader *reader);

    /**
     * @brief Read mesh buffer from XML
     * @param reader XML reader
     * @return Pointer to created mesh buffer
     */
    IMeshBuffer* readMeshBuffer(io::IXMLReader *reader);

    /**
     * @brief Skip unknown section in XML
     * @param reader XML reader
     * @param reportSkipping Whether to log skipping
     */
    void skipSection(io::IXMLReader *reader, bool reportSkipping);

    /**
     * @brief Read material from XML element
     * @param reader XML reader positioned at material element
     */
    void readMaterial(io::IXMLReader *reader);

    /**
     * @brief Parse float from character pointer
     * @param p Pointer to pointer to parse (will be advanced)
     * @return Parsed float value
     */
    inline f32 readFloat(const c8 **p);

    /**
     * @brief Parse integer from character pointer
     * @param p Pointer to pointer to parse (will be advanced)
     * @return Parsed integer value
     */
    inline s32 readInt(const c8 **p);

    /**
     * @brief Find next non-whitespace character
     * @param p Pointer to pointer to advance
     */
    void findNextNoneWhiteSpace(const c8 **p);

    /**
     * @brief Skip current non-whitespace token
     * @param p Pointer to pointer to advance
     */
    void skipCurrentNoneWhiteSpace(const c8 **p);

    /**
     * @brief Read array of floats from XML element
     * @param reader XML reader
     * @param floats Array to store floats
     * @param count Number of floats to read
     */
    void readFloatsInsideElement(io::IXMLReader *reader, f32 *floats, u32 count);

    /**
     * @brief Read mesh buffer vertices
     * @param reader XML reader
     * @param vertexCount Number of vertices
     * @param sbuffer Dynamic mesh buffer to populate
     */
    void readMeshBuffer(io::IXMLReader *reader, int vertexCount, CDynamicMeshBuffer *sbuffer);

    /**
     * @brief Read index buffer
     * @param reader XML reader
     * @param indexCount Number of indices
     * @param indices Index buffer to populate
     */
    void readIndices(io::IXMLReader *reader, int indexCount, IIndexBuffer &indices);


    // member variables

    scene::ISceneManager *SceneManager;
    io::IFileSystem      *FileSystem;
};
}   // end namespace scene
} // end namespace irr
#endif