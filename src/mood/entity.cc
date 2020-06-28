#pragma once

#include "common/common.cc"
#include "physics/physics.cc"
#include "util/cooldown.cc"

namespace mood {

#define ENTITY_DECL                              \
  u32 id;                                        \
  u32 particle_id;                               \
  u32 flags;                                     \
  u32 type


#define DECLARE_ENTITY(entity_type, tid)            \
  entity_type*                                      \
  UseEntity##entity_type(v2f pos, v2f dims)         \
  {                                                 \
    Entity* e = UseEntity();                        \
    if (!e) return nullptr;                         \
    entity_type t = {};                             \
    t.id = e->id;                                   \
    memcpy(e, &t, sizeof(t));                       \
    physics::Particle2d* p =                        \
        physics::CreateParticle2d(pos, dims);       \
    e->particle_id = p->id;                         \
    p->entity_id = e->id;                           \
    return (entity_type*)e;                         \
  }                                                 \
                                                    \
  entity_type*                                      \
  i2##entity_type(uint64_t idx)                     \
  {                                                 \
    assert(idx < kUsedEntity);                      \
    entity_type* t = (entity_type*)(&kEntity[idx]); \
    if (t->type != tid) return nullptr;             \
    if (t->id == 0) return nullptr;                 \
    return t;                                       \
  }                                                 \
                                                    \
  entity_type*                                      \
  Find##entity_type(uint32_t id)                    \
  {                                                 \
    entity_type* t = (entity_type*)FindEntity(id);  \
    if (!t || t->type != tid) return nullptr;       \
    return t;                                       \
  }                                                 \
                                                    \
  physics::Particle2d*                              \
  FindParticle(entity_type* type)                   \
  {                                                 \
    return physics::FindParticle2d(                 \
        type->particle_id);                         \
  }                                                 \
                                                    \
  void                                              \
  SetDestroyFlag(entity_type* entity)               \
  {                                                 \
    SBIT(entity->flags, 0);                         \
  }                                                 \
                                                    \
  bool                                              \
  IsDestroyFlagSet(entity_type* entity)             \
  {                                                 \
    return FLAGGED(entity->flags, 0);               \
  }


#define FOR_EACH_ENTITY(type, vname, body)                          \
  {                                                                 \
    type* vname;                                                    \
    for (int Idx##type = 0; Idx##type < kUsedEntity; ++Idx##type) { \
      vname = i2##type(Idx##type);                                  \
      if (!vname) continue;                                         \
      body                                                          \
    }                                                               \
  }

#define FOR_EACH_ENTITY_P(type, vname, pname, body)                 \
  {                                                                 \
    type* vname;                                                    \
    for (int Idx##type = 0; Idx##type < kUsedEntity; ++Idx##type) { \
      vname = i2##type(Idx##type);                                  \
      if (!vname) continue;                                         \
      physics::Particle2d* pname = FindParticle(vname);             \
      if (!pname) continue;                                         \
      body                                                          \
    }                                                               \
  }


enum EntityType {
  kEntityTypeInvalid = 0,
  kEntityTypeCharacter = 1,
  kEntityTypeProjectile = 2,
};

enum EntityFlags {
  kEntityDestroy = 0,
};

enum CharacterFlags {
  // If set the character will attempt to fire their primary weapon.
  kCharacterFireWeapon = 0,
  // If set the character will attempt to jump.
  kCharacterJump = 1,
  // Set if the character is aiming at something.
  kCharacterAim = 2,
};

enum CharacterAbilityFlags {
  // If set the character will boost in the direction of its velocity.
  kCharacterAbilityBoost = 0,
};

enum CharacterBbEntry {
  kAIBbType = 0, // Stores value of CharacterAIBehavior.
};

enum CharacterAIBehavior {
  kBehaviorNull = 0,   // Blackboard entries can't be 0.
  kBehaviorSimple = 1,
  kBehaviorSimpleFlying = 2,
};

struct Character {
  ENTITY_DECL = kEntityTypeCharacter;
  v2f facing = {1.f, 0.f};
  u8 prev_character_flags = 0;
  u8 character_flags = 0;
  // Used for when a character executes an ability.
  u8 ability_flags = 0;
  v2f ability_dir = {};
  // Used to show a flashy trail effect when the player executes a boost.
  u8 trail_effect_ttl = 0;
  // AI knowledge.
  Blackboard bb;  // TODO: Consider making a map of these...
  // Character stats.
  r32 health = 10.f;
  r32 max_health = 10.f;
  // Use if character is aiming.
  v2f aim_dir = {0.f, 1.f};
  // How much the aim should rotate.
  r32 aim_rotate_delta = 0.f;
};

enum ProjectileType {
  kProjectileLaser = 0,
  kProjectileBullet = 1,
};

struct Projectile {
  ENTITY_DECL = kEntityTypeProjectile;
  v2f dir;
  ProjectileType projectile_type;
  // Number of updates the projectile should live for.
  uint64_t updates_to_live = 0;
  // Entity that fired the projectile.
  u32 from_entity = 0;
};

union Entity {
  struct {
    ENTITY_DECL;
  };
  Character character;
  Projectile projectile;

  Entity() : type(kEntityTypeInvalid) {}
};

DECLARE_HASH_ARRAY(Entity, 256);

DECLARE_ENTITY(Character, kEntityTypeCharacter);
DECLARE_ENTITY(Projectile, kEntityTypeProjectile);

}
