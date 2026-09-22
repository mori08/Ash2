#include <Siv3D.hpp>

#include "System/EnemyMotionSystem.hpp"

#include "Component/Attack.hpp"
#include "Component/Dead.hpp"
#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/Player.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"
#include "FrameData.hpp"

namespace EnemyMotion {

namespace {

/// @brief 索敵に使うプレイヤーの WorldPos を返す
/// @return プレイヤーが存在しない、または撃破済み（`Dead`）なら none
[[nodiscard]] Optional<WorldPos> FindPlayerPos(entt::registry& registry) {
  const auto view = registry.view<Player, WorldPos>(entt::exclude<Dead>);
  const auto entity = view.front();
  if (entity == entt::null) return none;
  return view.get<WorldPos>(entity);
}

/// @brief w-d 平面上の距離を返す
[[nodiscard]] double DistanceWD(const WorldPos& a, const WorldPos& b) {
  const double dw = a.w - b.w;
  const double dd = a.d - b.d;
  return Math::Sqrt(dw * dw + dd * dd);
}

/// @brief target との w 座標の比較で facingRight を更新する
///
/// 等しい場合は変更しない（Neutral の横速度による向き更新と同じ扱い）。
void UpdateFacing(
    SpriteAnimation& anim, const WorldPos& self, const WorldPos& target
) {
  if (target.w > self.w) {
    anim.facingRight = true;
  } else if (target.w < self.w) {
    anim.facingRight = false;
  }
}

}  // namespace

Optional<Variant> Tick(
    Idle& /*state*/, entt::registry& registry, entt::entity entity,
    const FrameData& /*frameData*/
) {
  const auto& pos = registry.get<WorldPos>(entity);
  if (!pos.isOnGround()) return none;

  const auto playerPos = FindPlayerPos(registry);
  if (!playerPos) return none;

  const auto& cfg = registry.ctx().get<EnemyConfig>();
  if (DistanceWD(pos, *playerPos) > cfg.aggroRange) return none;

  auto& anim = registry.get<SpriteAnimation>(entity);
  UpdateFacing(anim, pos, *playerPos);
  SetClip(anim, U"move");
  return Chase{};
}

Optional<Variant> Tick(
    Chase& /*state*/, entt::registry& registry, entt::entity entity,
    const FrameData& /*frameData*/
) {
  const auto& cfg = registry.ctx().get<EnemyConfig>();
  const auto& pos = registry.get<WorldPos>(entity);
  auto& anim = registry.get<SpriteAnimation>(entity);
  auto& vel = registry.get<Velocity>(entity);

  const auto playerPos = FindPlayerPos(registry);
  if (!playerPos) {
    // 索敵範囲外への移動・撃破などで見失った場合、Velocity
    // を残したままにすると徘徊し続けるため Idle へ戻す
    vel.w = 0.0;
    vel.d = 0.0;
    SetClip(anim, U"idle");
    return Idle{};
  }
  const WorldPos& target = *playerPos;

  UpdateFacing(anim, pos, target);

  const double dist = DistanceWD(pos, target);
  if (dist <= cfg.leapRange) {
    // Chase は毎フレーム Velocity を書くため、抜けるときに 0 へ戻す
    vel.w = 0.0;
    vel.d = 0.0;
    SetClip(anim, U"windup");
    return Windup{.remaining = cfg.windupSec};
  }

  const double invDist = 1.0 / dist;
  vel.w = (target.w - pos.w) * invDist * cfg.moveSpeed;
  vel.d = (target.d - pos.d) * invDist * cfg.moveSpeed;
  return none;
}

Optional<Variant> Tick(
    Windup& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  state.remaining -= frameData.dt;
  if (state.remaining > 0.0) return none;

  const auto& cfg = registry.ctx().get<EnemyConfig>();
  const auto& pos = registry.get<WorldPos>(entity);
  auto& anim = registry.get<SpriteAnimation>(entity);

  // プレイヤーへ向かう単位ベクトル（w-d 平面）。Chase
  // と同じ経路で求めるが、溜め中にプレイヤーが移動した分を
  // 跳躍方向へ反映するため Windup 満了時に取り直す
  Vec2 dir{anim.facingRight ? 1.0 : -1.0, 0.0};
  if (const auto playerPos = FindPlayerPos(registry)) {
    const WorldPos& target = *playerPos;
    UpdateFacing(anim, pos, target);
    const double dist = DistanceWD(pos, target);
    if (dist > 0.0) {
      dir = Vec2{target.w - pos.w, target.d - pos.d} / dist;
    }
  }

  auto& vel = registry.get<Velocity>(entity);
  vel.w = dir.x * cfg.leapSpeedW;
  vel.d = dir.y * cfg.leapSpeedW;
  vel.h = cfg.leapSpeedH;

  SetClip(anim, U"leap");

  registry.emplace<Attack>(
      entity,
      Attack{
          .damage = cfg.attackDamage,
          .hitstopSec = cfg.attackHitstopSec,
          .reaction = cfg.attackReaction,
      }
  );

  return Leap{};
}

Optional<Variant> Tick(
    Leap& /*state*/, entt::registry& registry, entt::entity entity,
    const FrameData& /*frameData*/
) {
  const auto& pos = registry.get<WorldPos>(entity);
  auto& vel = registry.get<Velocity>(entity);
  if (!pos.isOnGround() || vel.h > 0.0) return none;

  const auto& cfg = registry.ctx().get<EnemyConfig>();
  registry.remove<Attack>(entity);
  vel.w = 0.0;
  vel.d = 0.0;

  SetClip(registry.get<SpriteAnimation>(entity), U"landing");

  return Landing{.remaining = cfg.landingSec};
}

Optional<Variant> Tick(
    Landing& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  state.remaining -= frameData.dt;
  if (state.remaining > 0.0) return none;

  SetClip(registry.get<SpriteAnimation>(entity), U"idle");
  return Idle{};
}

Optional<Variant> Tick(
    Stagger& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  state.remaining -= frameData.dt;
  if (state.remaining > 0.0) return none;

  SetClip(registry.get<SpriteAnimation>(entity), U"idle");
  return Idle{};
}

Optional<Variant> Tick(
    Repel& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  state.remaining -= frameData.dt;

  if (state.remaining <= 0.0) {
    registry.get<Velocity>(entity).w = 0.0;
    SetClip(registry.get<SpriteAnimation>(entity), U"idle");
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
    SetClip(registry.get<SpriteAnimation>(entity), U"idle");
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
