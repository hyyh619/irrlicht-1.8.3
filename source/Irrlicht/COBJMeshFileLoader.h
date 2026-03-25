// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OBJ_MESH_FILE_LOADER_H_INCLUDED__
#define __C_OBJ_MESH_FILE_LOADER_H_INCLUDED__

#include "IMeshLoader.h"
#include "IFileSystem.h"
#include "ISceneManager.h"
#include "irrString.h"
#include "SMeshBuffer.h"
#include "irrMap.h"

namespace irr
{
namespace scene
{
//! Meshloader capable of loading obj meshes.
class COBJMeshFileLoader : public IMeshLoader
{
public:

    /**
     * @brief Constructor
     * @param smgr Pointer to the scene manager
     * @param fs Pointer to the file system
     */
    COBJMeshFileLoader(scene::ISceneManager *smgr, io::IFileSystem *fs);

    /**
     * @brief Destructor
     */
    virtual ~COBJMeshFileLoader();

    /**
     * @brief Check if the file can be loaded based on extension
     * @param filename The file name to check
     * @return true if the file extension is loadable by this loader
     */
    virtual bool isALoadableFileExtension(const io::path &filename) const;

    /**
     * @brief Create an animated mesh from the OBJ file
     * @param file Pointer to the file to load
     * @return Pointer to the created animated mesh, or 0 if loading failed
     * @note The returned mesh must be dropped by the caller when no longer needed
     * @see IAnimatedMesh::drop()
     */
    virtual IAnimatedMesh* createMesh(io::IReadFile *file);

private:

    /**
     * @brief Material structure for OBJ files
     */
    struct SObjMtl
    {
        SObjMtl() : Meshbuffer(0), Bumpiness (1.0f), Illumination(0),
            RecalculateNormals(false)
        {
            Meshbuffer                         = new SMeshBuffer();
            Meshbuffer->Material.Shininess     = 0.0f;
            Meshbuffer->Material.AmbientColor  = video::SColorf(0.2f, 0.2f, 0.2f, 1.0f).toSColor();
            Meshbuffer->Material.DiffuseColor  = video::SColorf(0.8f, 0.8f, 0.8f, 1.0f).toSColor();
            Meshbuffer->Material.SpecularColor = video::SColorf(1.0f, 1.0f, 1.0f, 1.0f).toSColor();
        }

        SObjMtl(const SObjMtl &o)
            : Name(o.Name), Group(o.Group),
            Bumpiness(o.Bumpiness), Illumination(o.Illumination),
            RecalculateNormals(false)
        {
            Meshbuffer           = new SMeshBuffer();
            Meshbuffer->Material = o.Meshbuffer->Material;
        }

        core::map<video::S3DVertex, int> VertMap;           ///< Vertex map for deduplication
        scene::SMeshBuffer               *Meshbuffer;       ///< Mesh buffer holding geometry
        core::stringc                    Name;             ///< Material name
        core::stringc                    Group;            ///< Group name
        f32                              Bumpiness;        ///< Bumpiness factor
        c8                               Illumination;     ///< Illumination model
        bool                             RecalculateNormals; ///< Whether to recalculate normals
    };

    /**
     * @brief Read texture information from buffer
     * @param bufPtr Current buffer position
     * @param bufEnd End of buffer
     * @param currMaterial Current material to populate
     * @param relPath Relative path for textures
     * @return Pointer to next unprocessed position
     */
    const c8* readTextures(const c8 *bufPtr, const c8* const bufEnd, SObjMtl *currMaterial, const io::path &relPath);

    /**
     * @brief Move to first printable character
     * @param buf Buffer to process
     * @param bufEnd End of buffer
     * @param acrossNewlines Whether to cross newlines
     * @return Pointer to first word
     */
    const c8* goFirstWord(const c8 *buf, const c8* const bufEnd, bool acrossNewlines = true);
    
    /**
     * @brief Move to next word
     * @param buf Buffer to process
     * @param bufEnd End of buffer
     * @param acrossNewlines Whether to cross newlines
     * @return Pointer to next word
     */
    const c8* goNextWord(const c8 *buf, const c8* const bufEnd, bool acrossNewlines = true);
    
    /**
     * @brief Move to next line
     * @param buf Buffer to process
     * @param bufEnd End of buffer
     * @return Pointer to next line
     */
    const c8* goNextLine(const c8 *buf, const c8* const bufEnd);
    
    /**
     * @brief Copy current word to output buffer
     * @param outBuf Output buffer
     * @param inBuf Input buffer
     * @param outBufLength Maximum output length
     * @param pBufEnd End of input buffer
     * @return Number of characters copied
     */
    u32 copyWord(c8 *outBuf, const c8 *inBuf, u32 outBufLength, const c8* const pBufEnd);
    
    /**
     * @brief Copy current line to string
     * @param inBuf Input buffer
     * @param bufEnd End of buffer
     * @return The copied line
     */
    core::stringc copyLine(const c8 *inBuf, const c8* const bufEnd);

    /**
     * @brief Move to next word and copy it
     * @param outBuf Output buffer
     * @param inBuf Input buffer
     * @param outBufLength Maximum output length
     * @param pBufEnd End of input buffer
     * @return Pointer after copied word
     */
    const c8* goAndCopyNextWord(c8 *outBuf, const c8 *inBuf, u32 outBufLength, const c8* const pBufEnd);

    /**
     * @brief Read material from MTL file
     * @param fileName Name of the material file
     * @param relPath Relative path for texture resolution
     */
    void readMTL(const c8 *fileName, const io::path &relPath);

    /**
     * @brief Find material by name
     * @param mtlName Material name to find
     * @param grpName Group name to search
     * @return Pointer to found material, or 0
     */
    SObjMtl* findMtl(const core::stringc &mtlName, const core::stringc &grpName);

    /**
     * @brief Read RGB color from buffer
     * @param bufPtr Current buffer position
     * @param color Color to populate
     * @param pBufEnd End of buffer
     * @return Pointer after parsed data
     */
    const c8* readColor(const c8 *bufPtr, video::SColor &color, const c8* const pBufEnd);
    
    /**
     * @brief Read 3D vector from buffer
     * @param bufPtr Current buffer position
     * @param vec Vector to populate
     * @param pBufEnd End of buffer
     * @return Pointer after parsed data
     */
    const c8* readVec3(const c8 *bufPtr, core::vector3df &vec, const c8* const pBufEnd);
    
    /**
     * @brief Read 2D vector from buffer
     * @param bufPtr Current buffer position
     * @param vec Vector to populate
     * @param pBufEnd End of buffer
     * @return Pointer after parsed data
     */
    const c8* readUV(const c8 *bufPtr, core::vector2df &vec, const c8* const pBufEnd);
    
    /**
     * @brief Read boolean value
     * @param bufPtr Current buffer position
     * @param tf Boolean to populate
     * @param bufEnd End of buffer
     * @return Pointer after parsed data
     */
    const c8* readBool(const c8 *bufPtr, bool &tf, const c8* const bufEnd);

    /**
     * @brief Parse vertex indices from face data
     * @param vertexData Face vertex data
     * @param idx Array to store indices (vertex, UV, normal)
     * @param bufEnd End of buffer
     * @param vbsize Vertex buffer size
     * @param vtsize UV buffer size
     * @param vnsize Normal buffer size
     * @return true if successful
     */
    bool retrieveVertexIndices(c8 *vertexData, s32 *idx, const c8 *bufEnd, u32 vbsize, u32 vtsize, u32 vnsize);

    void cleanUp();

    scene::ISceneManager *SceneManager;
    io::IFileSystem      *FileSystem;

    core::array<SObjMtl*> Materials;
};
}   // end namespace scene
} // end namespace irr
#endif