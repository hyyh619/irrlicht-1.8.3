// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_3DS_MESH_FILE_LOADER_H_INCLUDED__
#define __C_3DS_MESH_FILE_LOADER_H_INCLUDED__

#include "IMeshLoader.h"
#include "IFileSystem.h"
#include "ISceneManager.h"
#include "irrString.h"
#include "SMesh.h"
#include "matrix4.h"

namespace irr
{
namespace scene
{
//! Meshloader capable of loading 3ds meshes.
class C3DSMeshFileLoader : public IMeshLoader
{
public:

    /**
     * @brief Constructor
     * @param smgr Pointer to the scene manager
     * @param fs Pointer to the file system
     */
    C3DSMeshFileLoader(ISceneManager *smgr, io::IFileSystem *fs);

    /**
     * @brief Destructor
     */
    virtual ~C3DSMeshFileLoader();

    /**
     * @brief Check if the file can be loaded based on extension
     * @param filename The file name to check
     * @return true if the file extension is loadable by this loader
     */
    virtual bool isALoadableFileExtension(const io::path &filename) const;

    /**
     * @brief Create an animated mesh from the 3DS file
     * @param file Pointer to the file to load
     * @return Pointer to the created animated mesh, or 0 if loading failed
     * @note The returned mesh must be dropped by the caller when no longer needed
     * @see IAnimatedMesh::drop()
     */
    virtual IAnimatedMesh* createMesh(io::IReadFile *file);

private:

// byte-align structures
#include "irrpack.h"

    /**
     * @brief Header structure for 3DS chunks
     */
    struct ChunkHeader
    {
        u16 id;      ///< Chunk identifier
        s32 length;  ///< Length of chunk data
    } PACK_STRUCT;

// Default alignment
#include "irrunpack.h"

    /**
     * @brief Container for chunk data during parsing
     */
    struct ChunkData
    {
        ChunkData() : read(0) {}

        ChunkHeader header;  ///< The chunk header
        s32         read;    ///< Number of bytes read
    };

    /**
     * @brief Material data for 3DS objects
     */
    struct SCurrentMaterial
    {
        void clear()
        {
            Material    = video::SMaterial();
            Name        = "";
            Filename[0] = "";
            Filename[1] = "";
            Filename[2] = "";
            Filename[3] = "";
            Filename[4] = "";
            Strength[0] = 0.f;
            Strength[1] = 0.f;
            Strength[2] = 0.f;
            Strength[3] = 0.f;
            Strength[4] = 0.f;
        }

        video::SMaterial Material;      ///< The material properties
        core::stringc    Name;           ///< Material name
        core::stringc    Filename[5];   ///< Texture filenames (up to 5)
        f32              Strength[5];   ///< Texture strengths
    };

    /**
     * @brief Material group representing faces with the same material
     */
    struct SMaterialGroup
    {
        SMaterialGroup() : faceCount(0), faces(0) {};

        SMaterialGroup(const SMaterialGroup &o)
        {
            *this = o;
        }

        ~SMaterialGroup()
        {
            clear();
        }

        void clear()
        {
            delete[] faces;
            faces     = 0;
            faceCount = 0;
        }

        void operator =(const SMaterialGroup &o)
        {
            MaterialName = o.MaterialName;
            faceCount    = o.faceCount;
            faces        = new u16[faceCount];

            for (u16 i = 0; i<faceCount; ++i)
                faces[i] = o.faces[i];
        }

        core::stringc MaterialName;  ///< Name of the material
        u16           faceCount;      ///< Number of faces using this material
        u16           *faces;         ///< Array of face indices
    };

    bool readChunk(io::IReadFile *file, ChunkData *parent);
    bool readMaterialChunk(io::IReadFile *file, ChunkData *parent);
    bool readFrameChunk(io::IReadFile *file, ChunkData *parent);
    bool readTrackChunk(io::IReadFile *file, ChunkData &data,
                        IMeshBuffer *mb, const core::vector3df &pivot);
    bool readObjectChunk(io::IReadFile *file, ChunkData *parent);
    bool readPercentageChunk(io::IReadFile *file, ChunkData *chunk, f32 &percentage);
    bool readColorChunk(io::IReadFile *file, ChunkData *chunk, video::SColor &out);

    void readChunkData(io::IReadFile *file, ChunkData &data);
    void readString(io::IReadFile *file, ChunkData &data, core::stringc &out);
    void readVertices(io::IReadFile *file, ChunkData &data);
    void readIndices(io::IReadFile *file, ChunkData &data);
    void readMaterialGroup(io::IReadFile *file, ChunkData &data);
    void readTextureCoords(io::IReadFile *file, ChunkData &data);

    void composeObject(io::IReadFile *file, const core::stringc &name);
    void loadMaterials(io::IReadFile *file);
    void cleanUp();

    scene::ISceneManager *SceneManager;
    io::IFileSystem      *FileSystem;

    f32                         *Vertices;
    u16                         *Indices;
    u32                         *SmoothingGroups;
    core::array<u16>            TempIndices;
    f32                         *TCoords;
    u16                         CountVertices;
    u16                         CountFaces; // = CountIndices/4
    u16                         CountTCoords;
    core::array<SMaterialGroup> MaterialGroups;

    SCurrentMaterial              CurrentMaterial;
    core::array<SCurrentMaterial> Materials;
    core::array<core::stringc>    MeshBufferNames;
    core::matrix4                 TransformationMatrix;

    SMesh *Mesh;
};
}   // end namespace scene
} // end namespace irr
#endif