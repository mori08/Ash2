#include <Siv3D.hpp>

#include "Component/Hierarchy.hpp"
#include "Component/ReactionLevel.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Phase/FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

namespace {

/// @brief
/// 攻撃フレーム内の進行度からダッシュ攻撃の珠オフセット（水平軌道）を返す
///
/// Vec3 の x が w 軸（横）、z が d 軸（奥行き）、y が高さ固定（capMidH）。
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
// progress / orbitRadius は隣接する double 引数として渡すと取り違えやすいため
// （bugprone-easily-swappable-parameters 対策）、orbitRadius は
// DashAttackConfig への const 参照経由で受け取る。
Vec3 DashAttackOrbOffset(double progress, const DashAttackConfig& da,
                         double capMidH) {
  const double angle = Math::TwoPi * progress;
  return Vec3{da.orbitRadius * Math::Cos(angle), capMidH,
              da.orbitRadius * Math::Sin(angle)};
}

}  // namespace

DashAttack MakeDashAttack(SpriteAnimation& anim, bool air, Vec2 dashDir) {
  SetClip(anim, U"dash_attack");
  return DashAttack{.elapsed = 0.0,
                    .air = air,
                    .hitboxEntity = entt::null,
                    .dashDir = dashDir};
}

Optional<Motion> Tick(DashAttack& state, entt::registry& registry,
                      entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& da = cfg.dashAttack;
  const auto& timeline = da.timeline;
  const auto& pos = registry.get<WorldPos>(entity);
  auto& vel = registry.get<Velocity>(entity);
  auto& anim = registry.get<SpriteAnimation>(entity);

  state.elapsed += frameData.dt;

  // 接地検出は後隙中も含め毎フレーム優先して評価する（タイマー満了判定より先、
  // 空中発動時のみ。地上 DashAttack は接地遷移を持たない）
  if (state.air && pos.isOnGround()) {
    if (state.hitboxEntity != entt::null) {
      Hierarchy::DestroyWithChildren(registry, state.hitboxEntity);
      state.hitboxEntity = entt::null;
    }
    return Landing{.timer = cfg.landing.recoverySec};
  }

  // 攻撃判定区間：ヒットボックスの生成・軌道更新・後隙以降の破棄
  const double capMidH = cfg.melee.capMidH;
  const auto offsetFn = [&da, capMidH](double progress) {
    return DashAttackOrbOffset(progress, da, capMidH);
  };
  UpdateAttackHitbox(registry, entity, state.elapsed, timeline,
                     HitboxSpec{.radius = da.radius,
                                .damage = da.damage,
                                .reaction = ReactionLevel::Repel,
                                .hitstopSec = da.hitstopSec},
                     state.hitboxEntity, offsetFn);

  // 突進フェーズ（構え）：ダッシュ方向へ移動
  // 空中発動時は AirDash の暫定仕様に合わせ垂直速度を 0 に固定する
  if (state.elapsed < timeline.activeStart()) {
    vel.w = state.dashDir.x * da.speed;
    vel.d = state.dashDir.y * da.speed;
    if (state.air) vel.h = 0.0;
  } else {
    vel.w = 0.0;
    vel.d = 0.0;
  }

  if (state.dashDir.x > 0.0) {
    anim.facingRight = true;
  } else if (state.dashDir.x < 0.0) {
    anim.facingRight = false;
  }

  if (timeline.isFinished(state.elapsed)) {
    return Neutral{};
  }

  return none;
}

}  // namespace PlayerMotion
