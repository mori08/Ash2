#include <Siv3D.hpp>

#include "Component/ReactionLevel.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

namespace {

/// @brief 攻撃フレーム内の進行度から空中攻撃の珠オフセット（垂直軌道）を返す
///
/// Vec3 の x が w 軸（横）、y が高さ（capMidH を中心に周回）、z は 0
/// 固定。DashAttack の w-d 平面軌道に対し、こちらは w-h 平面（垂直面）を
/// 周回する。回転方向はプレイヤーの向きに応じて左右反転する。
/// 角度は orbitStartDeg から orbitEndDeg へ線形に振る。
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
/// @param facingRight プレイヤーの向き（false なら w 成分の符号を反転）
// progress / orbitRadius は隣接する double 引数として渡すと取り違えやすいため
// （bugprone-easily-swappable-parameters 対策）、orbitRadius は
// AirAttackConfig への const 参照経由で受け取る。
Vec3 AirAttackOrbOffset(
    double progress, const AirAttackConfig& aa, double capMidH, bool facingRight
) {
  const double angle =
      Math::ToRadians(Math::Lerp(aa.orbitStartDeg, aa.orbitEndDeg, progress));
  const double w =
      facingRight ? aa.orbitRadius * Math::Cos(angle)
                  : -aa.orbitRadius * Math::Cos(angle);
  return Vec3{w, capMidH - aa.orbitRadius * Math::Sin(angle), 0.0};
}

}  // namespace

AirAttack MakeAirAttack(SpriteAnimation& anim) {
  SetClip(anim, U"air_attack");
  return AirAttack{.elapsed = 0.0};
}

Optional<Variant> Tick(
    AirAttack& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& aa = cfg.airAttack;
  const auto& timeline = aa.timeline;
  const auto& pos = registry.get<WorldPos>(entity);

  state.elapsed += frameData.dt;

  // 接地検出は後隙中も含め毎フレーム優先して評価する（タイマー満了判定より先）
  if (pos.isOnGround()) {
    StopHorizontalMovement(registry, entity);
    ReleaseAttackOrbs(registry, entity, cfg.attackEffect.fadeSec);
    return Landing{.timer = cfg.landing.recoverySec};
  }

  // ドリフト：構え・攻撃・後隙の全区間で移動入力に応じて水平移動する。
  // 珠の軌道が facingRight で左右反転するため、向きはここでは変えない
  auto& vel = registry.get<Velocity>(entity);
  const double driftSpeed = cfg.speed * aa.driftRatio;
  vel.w = frameData.input.moveAxis.x * driftSpeed;
  vel.d = frameData.input.moveAxis.y * driftSpeed;

  const auto& anim = registry.get<SpriteAnimation>(entity);
  const bool facingRight = anim.facingRight;
  const auto offsetFn = [&aa, &cfg, facingRight](double progress) {
    return AirAttackOrbOffset(progress, aa, cfg.melee.capMidH, facingRight);
  };
  UpdateAttackHitbox(
      registry, entity, state.elapsed, timeline,
      HitboxSpec{
          .radius = aa.radius,
          .damage = aa.damage,
          .reaction = ReactionLevel::Repel,
          .hitstopSec = aa.hitstopSec,
          .fadeSec = cfg.attackEffect.fadeSec
      },
      offsetFn
  );

  // 接地せずに終わった場合はタイマー満了で Neutral へ戻る
  if (timeline.isFinished(state.elapsed)) {
    return Neutral{};
  }

  return none;
}

}  // namespace PlayerMotion
