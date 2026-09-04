#include <Siv3D.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/LockOn.hpp"
#include "Component/Projectile.hpp"
#include "Component/Stamina.hpp"
#include "Component/Team.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "FrameData.hpp"
#include "System/LockOnSystem.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

namespace {

constexpr ColorF kBulletColor = {0.9, 0.9, 0.3};

/// @brief owner の LockOn.target へ向かう単位ベクトルを返す
/// @return ロック対象がなければ none
[[nodiscard]] Optional<Vec3> AimDirection(
    const entt::registry& registry, entt::entity owner, const WorldPos& spawnPos
) {
  const auto* lockOn = registry.try_get<LockOn>(owner);
  if (lockOn == nullptr || lockOn->target == entt::null ||
      !registry.valid(lockOn->target)) {
    return none;
  }

  const auto aim = LockOnSystem::AimPoint(
      registry.get<WorldPos>(lockOn->target),
      registry.get<Collider>(lockOn->target)
  );
  const Vec3 toAim{aim.w - spawnPos.w, aim.h - spawnPos.h, aim.d - spawnPos.d};
  if (toAim.isZero()) return none;

  return toAim.normalized();
}

}  // namespace

void SpawnProjectile(
    entt::registry& registry, entt::entity owner, const PlayerConfig& cfg,
    SpriteAnimation& anim
) {
  const auto& ownerPos = registry.get<WorldPos>(owner);
  const WorldPos spawnPos{
      .w = ownerPos.w, .h = ownerPos.h + cfg.ranged.spawnHeight, .d = ownerPos.d
  };

  // ロック中はその方向へ、そうでなければ従来どおり正面（facingRight）へ撃つ
  const double facingSign = anim.facingRight ? 1.0 : -1.0;
  const Vec3 dir =
      AimDirection(registry, owner, spawnPos)
          .value_or(Vec3{facingSign, 0.0, 0.0});

  if (dir.x > 0.0) {
    anim.facingRight = true;
  } else if (dir.x < 0.0) {
    anim.facingRight = false;
  }

  const auto bullet = registry.create();
  registry.emplace<Team>(bullet, Team::Player);
  registry.emplace<WorldPos>(bullet, spawnPos);
  registry.emplace<Velocity>(
      bullet,
      Velocity{
          .w = dir.x * cfg.ranged.bulletSpeed,
          .h = dir.y * cfg.ranged.bulletSpeed,
          .d = dir.z * cfg.ranged.bulletSpeed,
      }
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
  registry.emplace<Projectile>(
      bullet, Projectile{.origin = spawnPos, .maxRange = cfg.ranged.reach}
  );
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
