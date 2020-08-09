#pragma once

namespace mood {

const char*
SpawnerName(SpawnerType type)
{
  switch (type) {
    case kSpawnerPlayer: return "player";
    case kSpawnerSnail: return "snail";
    default: return "unknown";
  }
}

void
SpawnerCreate(v2f pos, SpawnerType type)
{
  switch (type) {
    case kSpawnerPlayer:
    case kSpawnerSnail: {
      ecs::Entity* entity = ecs::UseEntity();
      SpawnerComponent* spawner = ecs::AssignSpawnerComponent(entity);
      physics::Particle2d* p = physics::CreateParticle2d(
          pos, v2f(5.f, 5.f), entity->id);
      ecs::AssignPhysicsComponent(entity)->particle_id = p->id;
      spawner->spawner_type = type;
      SBIT(p->flags, physics::kParticleIgnoreGravity);
      SBIT(p->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(p->user_flags, kParticleSpawner);
    } break;
    default: {
      printf("%s Unknown spawner type.", __FUNCTION__);
      return;
    } break;
  }
}

void
SpawnerUpdate()
{
  ECS_ITR1(itr, kSpawnerComponent);
  while (itr.Next()) {
    SpawnerComponent* s = itr.c.spawner;
    physics::Particle2d* p = GetParticle(itr.e);
    if (s->spawn_count < s->spawn_to_count) {
      switch (s->spawner_type) {
        case kSpawnerPlayer: {
          if (!Player()) {
            PlayerCreate(p->position);
          }
        } break;
        case kSpawnerSnail: {
          AICreate(p->position, v2f(kEnemySnailWidth, kEnemySnailHeight),
                   kBehaviorSimple);
        } break;
        default: {
          printf("%s Unknown spawner type.", __FUNCTION__);
          return;
        } break;
      }
      ++s->spawn_count;
    }
  }
}

}
