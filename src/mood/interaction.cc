#pragma once

namespace mood {

// Turns constroller stick values into a facing direction and magnitude.
// Returns true if the stick movement is outside of a fixed value
// "dead zone"
bool
__CalculateStickMovement(
    s16 stick_x, s16 stick_y, v2f* dir, r32* normalized_magnitude)
{
  constexpr r32 kInputDeadzone = 4000.f;
  constexpr r32 kMaxControllerMagnitude = 32767.f;
  v2f stick(stick_x, stick_y);
  r32 magnitude = math::Length(stick);
  *dir = stick / magnitude;
  *normalized_magnitude = 0;
  if (magnitude > kInputDeadzone) {
    if (magnitude > kMaxControllerMagnitude) {
      magnitude = kMaxControllerMagnitude;
    }
    magnitude -= kInputDeadzone;
    *normalized_magnitude =
        magnitude / (kMaxControllerMagnitude - kInputDeadzone);
    
  } else {
    magnitude = 0.0;
    *normalized_magnitude = 0.0;
    return false;
  }
  return true;
}

void
ProcessPlatformEvent(const PlatformEvent& event, const v2f cursor)
{
  Character* player = Player();
  physics::Particle2d* particle = FindParticle(player);
  switch(event.type) {
    case KEY_DOWN: {
      switch (event.key) {
        case 27 /* ESC */: {
          exit(1);
        } break;
        case 'a': {
          SBIT(player->character_flags, kCharacterFireWeapon);
        } break;
        case 0 /* ARROW UP */: {
          SBIT(player->character_flags, kCharacterJump);
        } break;
        case 3 /* ARROW RIGHT */: {
          particle->acceleration.x = kPlayerAcceleration;
        } break;
        case 1 /* ARROW DOWN */: {
        } break;
        case 2 /* ARROW LEFT */: {
          particle->acceleration.x = -kPlayerAcceleration;
        } break;
        case 's': {
          SBIT(player->ability_flags, kCharacterAbilityBoost);
          player->ability_dir = math::Normalize(particle->velocity);
        } break;
      }
    } break;
    case KEY_UP: {
      switch (event.key) {
        case 'a': {
          CBIT(player->character_flags, kCharacterFireWeapon);
        } break;
        case 0  /* ARROW UP */: {
          CBIT(player->character_flags, kCharacterJump);
        } break;
        case 3 /* ARROW RIGHT */: {
          particle->acceleration.x = 0.f;
        } break;
        case 1 /* ARROW DOWN */: {
        } break;
        case 2 /* ARROW LEFT */: {
          particle->acceleration.x = 0.f;
        } break;
        case 's': {
          CBIT(player->ability_flags, kCharacterAbilityBoost);
        } break;
      }
    } break;
    case MOUSE_DOWN: {
      imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
      if (!imui::MouseInUI(cursor, imui::kEveryoneTag)) {
        physics::Particle2d* p = physics::CreateParticle2d(
            rgg::CameraRayFromMouseToWorld(cursor, 0.f).xy(),
            v2f(5.f, 5.f));
        p->inverse_mass = 0.f;
      }
    } break;
    case MOUSE_UP: {
      imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
    } break;
    case MOUSE_WHEEL: {
      imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
    } break;
    case XBOX_CONTROLLER: {
      v2f cdir;
      r32 cmag;
      if (__CalculateStickMovement(event.controller.lstick_x,
                                   event.controller.lstick_y, &cdir, &cmag)) {
        if (FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_X)) {
          SBIT(player->ability_flags, kCharacterAbilityBoost);
          player->ability_dir = cdir;
        }
        if (cdir.x > 0.f) {
          particle->acceleration.x = kPlayerAcceleration * cmag;
        } else if (cdir.x < 0.f) {
          particle->acceleration.x = -kPlayerAcceleration * cmag;
        }
      } else {
        particle->acceleration.x = 0.f;
      }

      if (!FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_X)) {
        CBIT(player->ability_flags, kCharacterAbilityBoost);
      }

      if (event.controller.left_trigger) {
        SBIT(player->character_flags, kCharacterAim);
      } else {
        CBIT(player->character_flags, kCharacterAim);
      }

      if (__CalculateStickMovement(event.controller.rstick_x,
                                   event.controller.rstick_y, &cdir, &cmag)) {
        if (FLAGGED(player->character_flags, kCharacterAim)) {
          if (cdir.x > 0.f) {
            player->aim_rotate_delta = -cmag;
          } else if (cdir.x < 0.f) {
            player->aim_rotate_delta = cmag;
          }
        }
      } else {
        player->aim_rotate_delta = 0.f;
      }

      if (FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_A)) {
        SBIT(player->character_flags, kCharacterJump);
      } else {
        CBIT(player->character_flags, kCharacterJump);
      }

      if (event.controller.right_trigger) {
        SBIT(player->character_flags, kCharacterFireWeapon);
      } else {
        CBIT(player->character_flags, kCharacterFireWeapon);
      }
    } break;
    default: break;
  }
}

void
EntityViewer(v2f screen)
{
  static char kUIBuffer[64];
  static b8 enable = false;
  static v2f pos(screen.x - 315, screen.y);
  imui::PaneOptions options;
  options.width = options.max_width = 315.f;
  imui::Begin("Entity Viewer", imui::kEveryoneTag, options, &pos, &enable);
  for (u32 i = 0; i < kUsedEntity; ++i) {
    Entity* e = &kEntity[i];
    imui::Indent(0);
    snprintf(kUIBuffer, sizeof(kUIBuffer), "Entity %u", e->id);
    imui::Text(kUIBuffer);
    imui::Indent(2);
    physics::Particle2d* p = physics::FindParticle2d(e->particle_id);
    snprintf(kUIBuffer, sizeof(kUIBuffer), "Particle %u", p->id);
    imui::Text(kUIBuffer);
    imui::Indent(4);
    snprintf(kUIBuffer, sizeof(kUIBuffer), "pos %.2f %.2f", p->position.x,
             p->position.y);
    imui::Text(kUIBuffer);
    snprintf(kUIBuffer, sizeof(kUIBuffer), "dims %.2f %.2f", p->dims.x,
             p->dims.y);
    imui::Text(kUIBuffer);
  };
  imui::End();
}

}
