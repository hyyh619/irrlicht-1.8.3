// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_PARTICLE_SYSTEM_SCENE_NODE_H_INCLUDED__
#define __C_PARTICLE_SYSTEM_SCENE_NODE_H_INCLUDED__

#include "IParticleSystemSceneNode.h"
#include "irrArray.h"
#include "irrList.h"
#include "SMeshBuffer.h"

namespace irr
{
namespace scene
{

/**
 * @brief Particle system scene node
 * 
 * Controls particle emission with various emitter types
 * (box, cylinder, mesh, point, ring, sphere) and affectors
 * (attraction, fade out, gravity, rotation, scale).
 */
class CParticleSystemSceneNode : public IParticleSystemSceneNode
{
public:

    /**
     * @brief Constructor
     * @param createDefaultEmitter Create default emitter
     * @param parent Parent scene node
     * @param mgr Scene manager
     * @param id Node ID
     * @param position Position
     * @param rotation Rotation
     * @param scale Scale
     */
    CParticleSystemSceneNode(bool createDefaultEmitter,
                             ISceneNode *parent, ISceneManager *mgr, s32 id,
                             const core::vector3df &position,
                             const core::vector3df &rotation,
                             const core::vector3df &scale);

    /**
     * @brief Destructor
     */
    virtual ~CParticleSystemSceneNode();

    /**
     * @brief Get particle emitter
     * @return Current particle emitter
     */
    virtual IParticleEmitter* getEmitter();

    /**
     * @brief Set particle emitter
     * @param emitter New emitter
     */
    virtual void setEmitter(IParticleEmitter *emitter);

    /**
     * @brief Add particle affector
     * @param affector Affector to add
     */
    virtual void addAffector(IParticleAffector *affector);

    /**
     * @brief Get all affectors
     * @return List of affectors
     */
    virtual const core::list<IParticleAffector*>&getAffectors() const;

    /**
     * @brief Remove all affectors
     */
    virtual void removeAllAffectors();

    /**
     * @brief Get material
     * @param i Material index
     * @return Material reference
     */
    virtual video::SMaterial&getMaterial(u32 i);

    /**
     * @brief Get material count
     * @return Number of materials
     */
    virtual u32 getMaterialCount() const;

    /**
     * @brief Called when registered to scene
     */
    virtual void OnRegisterSceneNode();

    /**
     * @brief Render particles
     */
    virtual void render();

    /**
     * @brief Get bounding box
     * @return Axis-aligned bounding box
     */
    virtual const core::aabbox3d<f32>&getBoundingBox() const;

    /**
     * @brief Create animated mesh emitter
     * @param node Animated mesh to emit from
     * @param useNormalDirection Use normal direction
     * @param direction Emission direction
     * @param normalDirectionModifier Normal direction strength
     * @param mbNumber Mesh buffer number
     * @param everyMeshVertex Emit from every vertex
     * @param minParticlesPerSecond Min particles per second
     * @param maxParticlesPerSecond Max particles per second
     * @param minStartColor Min start color
     * @param maxStartColor Max start color
     * @param lifeTimeMin Min lifetime (ms)
     * @param lifeTimeMax Max lifetime (ms)
     * @param maxAngleDegrees Max angle variation
     * @param minStartSize Min start size
     * @param maxStartSize Max start size
     * @return New emitter
     */
    virtual IParticleAnimatedMeshSceneNodeEmitter* createAnimatedMeshSceneNodeEmitter(
        scene::IAnimatedMeshSceneNode *node, bool useNormalDirection = true,
        const core::vector3df &direction = core::vector3df(0.0f, 0.03f, 0.0f),
        f32 normalDirectionModifier = 100.0f, s32 mbNumber = -1,
        bool everyMeshVertex = false, u32 minParticlesPerSecond = 5,
        u32 maxParticlesPerSecond = 10,
        const video::SColor &minStartColor = video::SColor(255, 0, 0, 0),
        const video::SColor &maxStartColor = video::SColor(255, 255, 255, 255),
        u32 lifeTimeMin = 2000, u32 lifeTimeMax = 4000,
        s32 maxAngleDegrees = 0,
        const core::dimension2df &minStartSize = core::dimension2df(5.0f, 5.0f),
        const core::dimension2df &maxStartSize = core::dimension2df(5.0f, 5.0f));

    /**
     * @brief Create box emitter
     * @param box Emission box volume
     * @param direction Emission direction
     * @param minParticlesPerSecond Min particles per second
     * @param maxParticlesPerSecond Max particles per second
     * @param minStartColor Min start color
     * @param maxStartColor Max start color
     * @param lifeTimeMin Min lifetime (ms)
     * @param lifeTimeMax Max lifetime (ms)
     * @param maxAngleDegrees Max angle variation
     * @param minStartSize Min start size
     * @param maxStartSize Max start size
     * @return New emitter
     */
    virtual IParticleBoxEmitter* createBoxEmitter(
        const core::aabbox3df &box = core::aabbox3d<f32>(-10, 0, -10, 5, 30, 10),
        const core::vector3df &direction = core::vector3df(0.0f, 0.03f, 0.0f),
        u32 minParticlesPerSecond = 5,
        u32 maxParticlesPerSecond = 10,
        const video::SColor &minStartColor = video::SColor(255, 0, 0, 0),
        const video::SColor &maxStartColor = video::SColor(255, 255, 255, 255),
        u32 lifeTimeMin = 2000, u32 lifeTimeMax = 4000,
        s32 maxAngleDegrees = 0,
        const core::dimension2df &minStartSize = core::dimension2df(5.0f, 5.0f),
        const core::dimension2df &maxStartSize = core::dimension2df(5.0f, 5.0f));

    /**
     * @brief Create cylinder emitter
     * @param center Cylinder center
     * @param radius Cylinder radius
     * @param normal Cylinder normal
     * @param length Cylinder length
     * @param outlineOnly Emit from outline only
     * @param direction Emission direction
     * @param minParticlesPerSecond Min particles per second
     * @param maxParticlesPersSecond Max particles per second
     * @param minStartColor Min start color
     * @param maxStartColor Max start color
     * @param lifeTimeMin Min lifetime (ms)
     * @param lifeTimeMax Max lifetime (ms)
     * @param maxAngleDegrees Max angle variation
     * @param minStartSize Min start size
     * @param maxStartSize Max start size
     * @return New emitter
     */
    virtual IParticleCylinderEmitter* createCylinderEmitter(
        const core::vector3df &center, f32 radius,
        const core::vector3df &normal, f32 length,
        bool outlineOnly = false, const core::vector3df &direction = core::vector3df(0.0f, 0.5f, 0.0f),
        u32 minParticlesPerSecond = 5, u32 maxParticlesPersSecond = 10,
        const video::SColor &minStartColor = video::SColor(255, 0, 0, 0),
        const video::SColor &maxStartColor = video::SColor(255, 255, 255, 255),
        u32 lifeTimeMin = 2000, u32 lifeTimeMax = 4000,
        s32 maxAngleDegrees = 0,
        const core::dimension2df &minStartSize = core::dimension2df(5.0f, 5.0f),
        const core::dimension2df &maxStartSize = core::dimension2df(5.0f, 5.0f));

    /**
     * @brief Create mesh emitter
     * @param mesh Mesh to emit from
     * @param useNormalDirection Use normal direction
     * @param direction Emission direction
     * @param normalDirectionModifier Normal direction strength
     * @param mbNumber Mesh buffer number
     * @param everyMeshVertex Emit from every vertex
     * @param minParticlesPerSecond Min particles per second
     * @param maxParticlesPerSecond Max particles per second
     * @param minStartColor Min start color
     * @param maxStartColor Max start color
     * @param lifeTimeMin Min lifetime (ms)
     * @param lifeTimeMax Max lifetime (ms)
     * @param maxAngleDegrees Max angle variation
     * @param minStartSize Min start size
     * @param maxStartSize Max start size
     * @return New emitter
     */
    virtual IParticleMeshEmitter* createMeshEmitter(
        scene::IMesh *mesh, bool useNormalDirection = true,
        const core::vector3df &direction = core::vector3df(0.0f, 0.03f, 0.0f),
        f32 normalDirectionModifier = 100.0f, s32 mbNumber = -1,
        bool everyMeshVertex = false,
        u32 minParticlesPerSecond = 5,
        u32 maxParticlesPerSecond = 10,
        const video::SColor &minStartColor = video::SColor(255, 0, 0, 0),
        const video::SColor &maxStartColor = video::SColor(255, 255, 255, 255),
        u32 lifeTimeMin = 2000, u32 lifeTimeMax = 4000,
        s32 maxAngleDegrees = 0,
        const core::dimension2df &minStartSize = core::dimension2df(5.0f, 5.0f),
        const core::dimension2df &maxStartSize = core::dimension2df(5.0f, 5.0f));

    /**
     * @brief Create point emitter
     * @param direction Emission direction
     * @param minParticlesPerSecond Min particles per second
     * @param maxParticlesPerSecond Max particles per second
     * @param minStartColor Min start color
     * @param maxStartColor Max start color
     * @param lifeTimeMin Min lifetime (ms)
     * @param lifeTimeMax Max lifetime (ms)
     * @param maxAngleDegrees Max angle variation
     * @param minStartSize Min start size
     * @param maxStartSize Max start size
     * @return New emitter
     */
    virtual IParticlePointEmitter* createPointEmitter(
        const core::vector3df &direction = core::vector3df(0.0f, 0.03f, 0.0f),
        u32 minParticlesPerSecond = 5,
        u32 maxParticlesPerSecond = 10,
        const video::SColor &minStartColor = video::SColor(255, 0, 0, 0),
        const video::SColor &maxStartColor = video::SColor(255, 255, 255, 255),
        u32 lifeTimeMin = 2000, u32 lifeTimeMax = 4000,
        s32 maxAngleDegrees = 0,
        const core::dimension2df &minStartSize = core::dimension2df(5.0f, 5.0f),
        const core::dimension2df &maxStartSize = core::dimension2df(5.0f, 5.0f));

    /**
     * @brief Create ring emitter
     * @param center Ring center
     * @param radius Ring radius
     * @param ringThickness Ring thickness
     * @param direction Emission direction
     * @param minParticlesPerSecond Min particles per second
     * @param maxParticlesPerSecond Max particles per second
     * @param minStartColor Min start color
     * @param maxStartColor Max start color
     * @param lifeTimeMin Min lifetime (ms)
     * @param lifeTimeMax Max lifetime (ms)
     * @param maxAngleDegrees Max angle variation
     * @param minStartSize Min start size
     * @param maxStartSize Max start size
     * @return New emitter
     */
    virtual IParticleRingEmitter* createRingEmitter(
        const core::vector3df &center, f32 radius, f32 ringThickness,
        const core::vector3df &direction = core::vector3df(0.0f, 0.03f, 0.0f),
        u32 minParticlesPerSecond = 5,
        u32 maxParticlesPerSecond = 10,
        const video::SColor &minStartColor = video::SColor(255, 0, 0, 0),
        const video::SColor &maxStartColor = video::SColor(255, 255, 255, 255),
        u32 lifeTimeMin = 2000, u32 lifeTimeMax = 4000,
        s32 maxAngleDegrees = 0,
        const core::dimension2df &minStartSize = core::dimension2df(5.0f, 5.0f),
        const core::dimension2df &maxStartSize = core::dimension2df(5.0f, 5.0f));

    /**
     * @brief Create sphere emitter
     * @param center Sphere center
     * @param radius Sphere radius
     * @param direction Emission direction
     * @param minParticlesPerSecond Min particles per second
     * @param maxParticlesPerSecond Max particles per second
     * @param minStartColor Min start color
     * @param maxStartColor Max start color
     * @param lifeTimeMin Min lifetime (ms)
     * @param lifeTimeMax Max lifetime (ms)
     * @param maxAngleDegrees Max angle variation
     * @param minStartSize Min start size
     * @param maxStartSize Max start size
     * @return New emitter
     */
    virtual IParticleSphereEmitter* createSphereEmitter(
        const core::vector3df &center, f32 radius,
        const core::vector3df &direction = core::vector3df(0.0f, 0.03f, 0.0f),
        u32 minParticlesPerSecond = 5,
        u32 maxParticlesPerSecond = 10,
        const video::SColor &minStartColor = video::SColor(255, 0, 0, 0),
        const video::SColor &maxStartColor = video::SColor(255, 255, 255, 255),
        u32 lifeTimeMin = 2000, u32 lifeTimeMax = 4000,
        s32 maxAngleDegrees = 0,
        const core::dimension2df &minStartSize = core::dimension2df(5.0f, 5.0f),
        const core::dimension2df &maxStartSize = core::dimension2df(5.0f, 5.0f));

    /**
     * @brief Create attraction affector
     * @param point Attraction point
     * @param speed Attraction speed
     * @param attract Attract or repel
     * @param affectX Affect X axis
     * @param affectY Affect Y axis
     * @param affectZ Affect Z axis
     * @return New affector
     */
    virtual IParticleAttractionAffector* createAttractionAffector(
        const core::vector3df &point, f32 speed = 1.0f, bool attract = true,
        bool affectX = true, bool affectY = true, bool affectZ = true);

    /**
     * @brief Create scale affector
     * @param scaleTo Target scale
     * @return New affector
     */
    virtual IParticleAffector* createScaleParticleAffector(const core::dimension2df &scaleTo = core::dimension2df(1.0f, 1.0f));

    /**
     * @brief Create fade out affector
     * @param targetColor Target color
     * @param timeNeededToFadeOut Fade duration (ms)
     * @return New affector
     */
    virtual IParticleFadeOutAffector* createFadeOutParticleAffector(
        const video::SColor &targetColor = video::SColor(0, 0, 0, 0),
        u32 timeNeededToFadeOut = 1000);

    /**
     * @brief Create gravity affector
     * @param gravity Gravity vector
     * @param timeForceLost Time to lose force (ms)
     * @return New affector
     */
    virtual IParticleGravityAffector* createGravityAffector(
        const core::vector3df &gravity = core::vector3df(0.0f, -0.03f, 0.0f),
        u32 timeForceLost = 1000);

    /**
     * @brief Create rotation affector
     * @param speed Rotation speed (degrees/sec)
     * @param pivotPoint Pivot point
     * @return New affector
     */
    virtual IParticleRotationAffector* createRotationAffector(
        const core::vector3df &speed = core::vector3df(5.0f, 5.0f, 5.0f),
        const core::vector3df &pivotPoint = core::vector3df(0.0f, 0.0f, 0.0f));

    /**
     * @brief Set particle size
     * @param size Particle size
     */
    virtual void setParticleSize(
        const core::dimension2d<f32> &size = core::dimension2d<f32>(5.0f, 5.0f));

    /**
     * @brief Set particles global
     * @param global true = affected by node movement
     */
    virtual void setParticlesAreGlobal(bool global = true);

    /**
     * @brief Clear all particles
     */
    virtual void clearParticles();

    /**
     * @brief Manual particle update
     * @param time Time delta (ms)
     */
    virtual void doParticleSystem(u32 time);

    /**
     * @brief Serialize attributes
     * @param out Output attributes
     * @param options Read/write options
     */
    virtual void serializeAttributes(io::IAttributes *out, io::SAttributeReadWriteOptions *options = 0) const;

    /**
     * @brief Deserialize attributes
     * @param in Input attributes
     * @param options Read/write options
     */
    virtual void deserializeAttributes(io::IAttributes *in, io::SAttributeReadWriteOptions *options = 0);

    /**
     * @brief Get node type
     * @return Scene node type
     */
    virtual ESCENE_NODE_TYPE getType() const
    {
        return ESNT_PARTICLE_SYSTEM;
    }

private:

    /**
     * @brief Reallocate particle buffers
     */
    void reallocateBuffers();

    core::list<IParticleAffector*> AffectorList;  ///< List of affectors
    IParticleEmitter               *Emitter;    ///< Current emitter
    core::array<SParticle>         Particles;   ///< Particle data
    core::dimension2d<f32>         ParticleSize; ///< Particle size
    u32                            LastEmitTime;  ///< Last emit time
    s32                            MaxParticles;  ///< Max particles

    SMeshBuffer *Buffer;  ///< Mesh buffer for rendering

    enum E_PARTICLES_PRIMITIVE
    {
        EPP_POINT = 0,
        EPP_BILLBOARD,
        EPP_POINTSPRITE
    };
    E_PARTICLES_PRIMITIVE ParticlePrimitive;  ///< Render primitive type

    bool ParticlesAreGlobal;  ///< Global particles flag
};
}   // end namespace scene
} // end namespace irr
#endif