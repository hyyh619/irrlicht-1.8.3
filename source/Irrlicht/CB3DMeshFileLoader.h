// Copyright (C) 2006-2012 Luke Hoschke
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

// B3D Mesh loader
// File format designed by Mark Sibly for the Blitz3D engine and has been
// declared public domain

#include "IrrCompileConfig.h"

#ifndef __C_B3D_MESH_LOADER_H_INCLUDED__
#define __C_B3D_MESH_LOADER_H_INCLUDED__

#include "IMeshLoader.h"
#include "ISceneManager.h"
#include "CSkinnedMesh.h"
#include "IReadFile.h"

namespace irr
{
namespace scene
{
//! Meshloader for B3D format
class CB3DMeshFileLoader : public IMeshLoader
{
public:

    /**
     * @brief Constructor
     * @param smgr Pointer to the scene manager
     */
    CB3DMeshFileLoader(scene::ISceneManager *smgr);

    /**
     * @brief Check if the file can be loaded based on extension
     * @param filename The file name to check
     * @return true if the file extension is loadable by this loader
     */
    virtual bool isALoadableFileExtension(const io::path &filename) const;

    /**
     * @brief Create an animated mesh from the B3D file
     * @param file Pointer to the file to load
     * @return Pointer to the created animated mesh, or 0 if loading failed
     * @note The returned mesh must be dropped by the caller when no longer needed
     * @see IAnimatedMesh::drop()
     */
    virtual IAnimatedMesh* createMesh(io::IReadFile *file);

private:

    /**
     * @brief B3D chunk header structure
     */
    struct SB3dChunkHeader
    {
        c8  name[4];  ///< Chunk name (4 characters)
        s32 size;     ///< Chunk size
    };

    /**
     * @brief B3D chunk data structure
     */
    struct SB3dChunk
    {
        SB3dChunk(const SB3dChunkHeader &header, long sp)
            : length(header.size + 8), startposition(sp)
        {
            name[0] = header.name[0];
            name[1] = header.name[1];
            name[2] = header.name[2];
            name[3] = header.name[3];
        }

        c8   name[4];          ///< Chunk name
        s32  length;          ///< Chunk length
        long startposition;   ///< Starting position in file
    };

    /**
     * @brief B3D texture information
     */
    struct SB3dTexture
    {
        core::stringc TextureName;  ///< Texture filename
        s32           Flags;         ///< Texture flags
        s32           Blend;         ///< Blend mode
        f32           Xpos;          ///< X position
        f32           Ypos;          ///< Y position
        f32           Xscale;        ///< X scale
        f32           Yscale;        ///< Y scale
        f32           Angle;         ///< Rotation angle
    };

    /**
     * @brief B3D material structure
     */
    struct SB3dMaterial
    {
        SB3dMaterial() : red(1.0f), green(1.0f),
            blue(1.0f), alpha(1.0f), shininess(0.0f), blend(1),
            fx(0)
        {
            for (u32 i = 0; i<video::MATERIAL_MAX_TEXTURES; ++i)
                Textures[i] = 0;
        }
        video::SMaterial Material;                    ///< Irrlicht material
        f32              red, green, blue, alpha;     ///< Material colors
        f32              shininess;                   ///< Shininess factor
        s32              blend, fx;                   ///< Blend mode and effects
        SB3dTexture      *Textures[video::MATERIAL_MAX_TEXTURES]; ///< Textures
    };

    bool load();
    bool readChunkNODE(CSkinnedMesh::SJoint *InJoint);
    bool readChunkMESH(CSkinnedMesh::SJoint *InJoint);
    bool readChunkVRTS(CSkinnedMesh::SJoint *InJoint);
    bool readChunkTRIS(scene::SSkinMeshBuffer *MeshBuffer, u32 MeshBufferID, s32 Vertices_Start);
    bool readChunkBONE(CSkinnedMesh::SJoint *InJoint);
    bool readChunkKEYS(CSkinnedMesh::SJoint *InJoint);
    bool readChunkANIM();
    bool readChunkTEXS();
    bool readChunkBRUS();

    void loadTextures(SB3dMaterial &material) const;

    void readString(core::stringc &newstring);
    void readFloats(f32 *vec, u32 count);

    core::array<SB3dChunk> B3dStack;

    core::array<SB3dMaterial> Materials;
    core::array<SB3dTexture>  Textures;

    core::array<s32> AnimatedVertices_VertexID;

    core::array<s32> AnimatedVertices_BufferID;

    core::array<video::S3DVertex2TCoords> BaseVertices;

    ISceneManager *SceneManager;
    CSkinnedMesh  *AnimatedMesh;
    io::IReadFile *B3DFile;

    // B3Ds have Vertex ID's local within the mesh I don't want this
    // Variable needs to be class member due to recursion in calls
    u32 VerticesStart;

    bool NormalsInFile;
    bool HasVertexColors;
    bool ShowWarning;
};
}   // end namespace scene
} // end namespace irr
#endif // __C_B3D_MESH_LOADER_H_INCLUDED__