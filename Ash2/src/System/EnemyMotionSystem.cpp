#include <Siv3D.hpp>

#include "System/EnemyMotionSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"
#include "FrameData.hpp"

namespace EnemyMotion {

namespace {

/// @brief ひるみ表現の最大縮小率（縦方向）
constexpr double kMaxShrinkRatio = 0.2;

}  // namespace

Optional<Variant> Tick(
    Idle& /*state*/, entt::registry& /*registry*/, entt::entity /*entity*/,
    const FrameData& /*frameData*/
) {
  return none;
}

Optional<Variant> Tick(
    Stagger& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  const auto& cfg = registry.ctx().get<EnemyConfig>();
  state.remaining -= frameData.dt;

  if (auto* drawable = registry.try_get<Drawable>(entity);
      drawable != nullptr) {
    if (auto* rect = std::get_if<RectDrawable>(drawable); rect != nullptr) {
      if (state.remaining <= 0.0) {
        rect->size = cfg.size;
      } else {
        // duration の中間で最も縮み、両端（開始・終了）で原寸に近づく
        const double progress = state.remaining / cfg.staggerSec;
        const double shrink =
            (1.0 - Abs(progress * 2.0 - 1.0)) * kMaxShrinkRatio;
        rect->size.y = cfg.size.y * (1.0 - shrink);
      }
    }
  }

  if (state.remaining <= 0.0) {
    return Idle{};
  }
  return none;
}

Optional<Variant> Tick(
    Repel& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  state.remaining -= frameData.dt;

  if (state.remaining <= 0.0) {
    registry.get<Velocity>(entity).w = 0.0;
    return Idle{};
  }
  return none;
}

Optional<Variant> Tick(
    Knockback& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  state.remaining -= frameData.dt;

  // 打ち上げ直後は接地したままこの Tick に入るため、上昇中は止めない。
  // 横移動を止めるのは GravitySystem が vel.h をクランプした着地後のみ
  auto& vel = registry.get<Velocity>(entity);
  if (registry.get<WorldPos>(entity).isOnGround() && vel.h <= 0.0) {
    vel.w = 0.0;
  }

  if (state.remaining <= 0.0) {
    return Idle{};
  }
  return none;
}

Optional<Variant> Tick(
    Defeated& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  const auto& cfg = registry.ctx().get<EnemyConfig>();
  state.remaining -= frameData.dt;

  registry.get_or_emplace<DrawColor>(entity).color.a =
      Max(0.0, state.remaining / cfg.defeatedSec);

  return none;
}

}  // namespace EnemyMotion
