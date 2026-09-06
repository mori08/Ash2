#include <Siv3D.hpp>

#include "Component/Stamina.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

Optional<Variant> Tick(
    Neutral& /*state*/, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  const auto& input = frameData.input;
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& pos = registry.get<WorldPos>(entity);
  auto& vel = registry.get<Velocity>(entity);
  auto& anim = registry.get<SpriteAnimation>(entity);

  // 移動・ジャンプ・向き
  vel.w = input.moveAxis.x * cfg.speed;
  vel.d = input.moveAxis.y * cfg.speed;

  if (vel.w > 0.0) {
    anim.facingRight = true;
  } else if (vel.w < 0.0) {
    anim.facingRight = false;
  }

  // MeleeChain/Ranged への入場（接地中のみ）
  if (pos.isOnGround()) {
    if (input.attackDown &&
        registry.get<Stamina>(entity).current >=
            cfg.melee.chain[0].staminaCost) {
      // 直前で設定した横方向速度を打ち消す（持ち越すと1フレーム分滑る）
      vel.w = 0.0;
      vel.d = 0.0;
      // ヒットボックス・光は攻撃フレーム開始時に Tick(MeleeChain&, ...)
      // が生成する
      return MakeMeleeChain(registry, entity, anim, 0);
    }
    if (input.rangedAttackDown &&
        registry.get<Stamina>(entity).current >= cfg.ranged.staminaCost) {
      vel.w = 0.0;
      vel.d = 0.0;
      SpawnProjectile(registry, entity, cfg, anim);
      return MakeRanged(registry, entity, cfg, anim);
    }
    if (input.dashDown &&
        registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
      return MakeDash(registry, entity, cfg, anim, /*air=*/false);
    }
  } else if (
      input.attackDown &&
      registry.get<Stamina>(entity).current >= cfg.airAttack.staminaCost
  ) {
    // 空中攻撃への入場（AirAttack::Tick が接地検出で Landing へ遷移させる）
    vel.w = 0.0;
    vel.d = 0.0;
    return MakeAirAttack(registry, entity, anim);
  } else if (
      input.rangedAttackDown &&
      registry.get<Stamina>(entity).current >= cfg.ranged.staminaCost
  ) {
    // 空中遠距離攻撃への入場。Ranged は地上・空中で共有するため、
    // 着地しても Landing を挟まずタイマー満了で Neutral
    // に戻る（地上と同一挙動）。
    vel.w = 0.0;
    vel.d = 0.0;
    SpawnProjectile(registry, entity, cfg, anim);
    return MakeRanged(registry, entity, cfg, anim);
  } else if (
      input.dashDown &&
      registry.get<Stamina>(entity).current >= cfg.dash.staminaCost
  ) {
    // 空中ダッシュへの入場（Dash::Tick が air フラグにより接地検出で Landing
    // へ遷移させる）
    return MakeDash(registry, entity, cfg, anim, /*air=*/true);
  }

  if (input.jumpDown && pos.isOnGround()) {
    vel.h = cfg.jumpSpeed;
  }

  // ロコモーションクリップ（idle/move/jump_rise/jump_fall）
  // pos は前フレームまでの値のため、ジャンプ入力で vel.h を設定した
  // 直後のフレームでは isOnGround() がまだ true のままになる。
  // vel.h > 0.0（ジャンプによる上昇）の場合のみ判定に加えることで、
  // 重力による接地中の微小な負の vel.h には反応せず、
  // 入力と同フレームで jump_rise クリップに切り替える。
  if (!pos.isOnGround() || vel.h > 0.0) {
    SetClip(anim, vel.h > 0.0 ? U"jump_rise" : U"jump_fall");
  } else if (vel.w != 0.0 || vel.d != 0.0) {
    SetClip(anim, U"move");
  } else {
    SetClip(anim, U"idle");
  }

  return none;
}

}  // namespace PlayerMotion
