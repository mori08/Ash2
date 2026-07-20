#include <Siv3D.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Projectile.hpp"
#include "Component/Stamina.hpp"
#include "Component/Velocity.hpp"
#include "Phase/FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

namespace {

constexpr ColorF KBulletColor = {0.9, 0.9, 0.3};

/// @brief 指定クリップの再生時間（秒）を返す
/// @return クリップが見つからない場合は 0.0
double GetClipDuration(const AnimationData& data, const String& clip) {
  const auto it = data.clips.find(clip);
  if (it == data.clips.end()) return 0.0;
  return static_cast<double>(it->second.count) / it->second.speed;
}

}  // namespace

void SpawnProjectile(entt::registry& registry, const WorldPos& pos,
                     bool facingRight, const PlayerConfig& cfg) {
  const double sign = facingRight ? 1.0 : -1.0;
  const auto bullet = registry.create();
  registry.emplace<WorldPos>(
      bullet,
      WorldPos{.w = pos.w, .h = pos.h + cfg.ranged.spawnHeight, .d = pos.d});
  registry.emplace<Velocity>(bullet,
                             Velocity{.w = sign * cfg.ranged.bulletSpeed});
  registry.emplace<Collider>(bullet, Collider{
                                         .segmentStart = Vec3{0.0, 0.0, 0.0},
                                         .segmentEnd = Vec3{0.0, 0.0, 0.0},
                                         .radius = cfg.ranged.radius,
                                     });
  registry.emplace<Attack>(bullet, Attack{.damage = cfg.ranged.damage});
  registry.emplace<Drawable>(bullet, CircleDrawable{.radius = cfg.ranged.radius,
                                                    .color = KBulletColor});
  registry.emplace<Projectile>(bullet);
}

Ranged MakeRanged(entt::registry& registry, entt::entity entity,
                  const PlayerConfig& cfg, const AnimationData& playerData,
                  SpriteAnimation& anim) {
  auto& stamina = registry.get<Stamina>(entity);
  stamina.current = Max(0, stamina.current - cfg.ranged.staminaCost);
  SetClip(anim, U"ranged_attack");
  return Ranged{.timer = GetClipDuration(playerData, U"ranged_attack")};
}

Optional<Motion> Tick(Ranged& state, entt::registry& registry,
                      entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    return Neutral{};
  }

  return none;
}

}  // namespace PlayerMotion
