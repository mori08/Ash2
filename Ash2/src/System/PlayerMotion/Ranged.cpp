#include <Siv3D.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/Projectile.hpp"
#include "Component/Stamina.hpp"
#include "Component/Team.hpp"
#include "Component/Velocity.hpp"
#include "FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

namespace {

constexpr ColorF kBulletColor = {0.9, 0.9, 0.3};

}  // namespace

void SpawnProjectile(
    entt::registry& registry, const WorldPos& pos, bool facingRight,
    const PlayerConfig& cfg
) {
  const double sign = facingRight ? 1.0 : -1.0;
  const auto bullet = registry.create();
  registry.emplace<Team>(bullet, Team::Player);
  registry.emplace<WorldPos>(
      bullet,
      WorldPos{.w = pos.w, .h = pos.h + cfg.ranged.spawnHeight, .d = pos.d}
  );
  registry.emplace<Velocity>(
      bullet, Velocity{.w = sign * cfg.ranged.bulletSpeed}
  );
  registry.emplace<Collider>(
      bullet,
      Collider{
          .segmentStart = Vec3{0.0, 0.0, 0.0},
          .segmentEnd = Vec3{0.0, 0.0, 0.0},
          .radius = cfg.ranged.radius,
      }
  );
  registry.emplace<Attack>(bullet, Attack{.damage = cfg.ranged.damage});
  registry.emplace<Drawable>(
      bullet, CircleDrawable{.radius = cfg.ranged.radius}
  );
  registry.emplace<DrawColor>(bullet, DrawColor{.color = kBulletColor});
  registry.emplace<Projectile>(bullet);
}

Ranged MakeRanged(
    entt::registry& registry, entt::entity entity, const PlayerConfig& cfg,
    SpriteAnimation& anim
) {
  auto& stamina = registry.get<Stamina>(entity);
  stamina.current = Max(0, stamina.current - cfg.ranged.staminaCost);

  SetClip(anim, U"ranged_attack");
  return Ranged{.timer = cfg.ranged.recoverySec};
}

Optional<Variant> Tick(
    Ranged& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  StopHorizontalMovement(registry, entity);

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    return Neutral{};
  }

  return none;
}

}  // namespace PlayerMotion
