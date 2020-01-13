
#include "math/math.cc"

#include "common.cc"

struct Transform {
  math::Vec3f position;
  math::Vec3f scale = math::Vec3f(1.f, 1.f, 1.f);
  math::Quatf orientation;
};

struct Asteroid {
  Transform transform;
};
DECLARE_GAME_TYPE(Asteroid, 8);

struct Command {
  enum Type {
    kNone = 0,
    kMine = 1,
    kMove = 2,
  };
  Type type;
  math::Vec2f destination;
};

DECLARE_GAME_TYPE(Command, 16);
struct Unit {
  Transform transform;
  Command command;
  int kind = 0;
};

DECLARE_GAME_TYPE(Unit, 8);
