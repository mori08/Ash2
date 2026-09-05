#include <Siv3D.hpp>

#include "Component/Invincible.hpp"
#include "Component/Stamina.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

Variant MakeDamaged(
    entt::registry& registry, entt::entity entity, const PlayerConfig& cfg,
    SpriteAnimation& anim, ReactionLevel reaction, double knockbackSign
) {
  ReleaseAttackOrbs(registry, entity, cfg.attackEffect.fadeSec);

  registry.get<Velocity>(entity) = Velocity{};
  registry.remove<Invincible>(entity);

  // 空中被弾は Lv によらず Knockback にする（空中の仰け反りは設計に
  // 記述がなく、Downed を経由しないと落下後の復帰先が決まらないため）
  const bool aerial = !registry.get<WorldPos>(entity).isOnGround();
  if (reaction == ReactionLevel::Blow || aerial) {
    auto& vel = registry.get<Velocity>(entity);
    vel.w = knockbackSign * cfg.damage.knockbackSpeedW;
    vel.h = cfg.damage.knockbackSpeedH;
    registry.emplace_or_replace<Invincible>(entity);
    SetClip(anim, U"knockback");
    return Knockback{};
  }

  SetClip(anim, U"stagger");
  return Stagger{.timer = cfg.damage.staggerSec};
}

Optional<Variant> Tick(
    Stagger& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& input = frameData.input;

  // Stagger
  // は全区間でキャンセルを受ける（スタミナ不足なら無視して硬直を続ける）
  if (input.dashDown &&
      registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
    auto& anim = registry.get<SpriteAnimation>(entity);
    return MakeDash(registry, entity, cfg, anim, /*air=*/false);
  }

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    return Neutral{};
  }
  return none;
}

Optional<Variant> Tick(
    Knockback& /*state*/, entt::registry& registry, entt::entity entity,
    const FrameData& /*frameData*/
) {
  registry.emplace_or_replace<Invincible>(entity);

  const auto& pos = registry.get<WorldPos>(entity);
  const auto& vel = registry.get<Velocity>(entity);
  if (pos.isOnGround() && vel.h <= 0.0) {
    const auto& cfg = registry.ctx().get<PlayerConfig>();
    auto& anim = registry.get<SpriteAnimation>(entity);
    StopHorizontalMovement(registry, entity);
    SetClip(anim, U"downed");
    return Downed{.timer = cfg.damage.downSec};
  }
  return none;
}

Optional<Variant> Tick(
    Downed& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  registry.emplace_or_replace<Invincible>(entity);

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    const auto& cfg = registry.ctx().get<PlayerConfig>();
    auto& anim = registry.get<SpriteAnimation>(entity);
    // 起き上がりは無敵ではないため（設計の「無敵が切れる」を満たす）除去する
    registry.remove<Invincible>(entity);
    SetClip(anim, U"get_up");
    return GetUp{.timer = cfg.damage.getUpSec};
  }
  return none;
}

Optional<Variant> Tick(
    GetUp& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& input = frameData.input;

  // GetUp は全区間でキャンセルを受ける（スタミナ不足なら無視して硬直を続ける）
  if (input.dashDown &&
      registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
    auto& anim = registry.get<SpriteAnimation>(entity);
    return MakeDash(registry, entity, cfg, anim, /*air=*/false);
  }

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    return Neutral{};
  }
  return none;
}

}  // namespace PlayerMotion
