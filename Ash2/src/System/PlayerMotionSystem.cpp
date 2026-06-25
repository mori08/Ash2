#include <Siv3D.hpp>

#include "System/PlayerMotionSystem.hpp"

#include <algorithm>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Invincible.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Stamina.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/AnimationData.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"

namespace PlayerMotion {

namespace {

constexpr s3d::ColorF KBulletColor = {0.9, 0.9, 0.3};
constexpr s3d::ColorF KMeleeOrbColor = {1.0, 0.9, 0.5};
/// 暫定のヒットストップ時間（秒）。本格的な数値調整は #132/#134 で行う
constexpr double KMeleeHitstopSec = 0.05;

/// @brief 指定クリップの再生時間（秒）を返す
/// @return クリップが見つからない場合は 0.0
double GetClipDuration(const AnimationData& data, const s3d::String& clip) {
  const auto it = data.clips.find(clip);
  if (it == data.clips.end()) return 0.0;
  return static_cast<double>(it->second.count) / it->second.speed;
}

/// @brief 横方向の速度を止める
void StopHorizontalMovement(entt::registry& registry, entt::entity entity) {
  auto& vel = registry.get<Velocity>(entity);
  vel.w = 0.0;
  vel.d = 0.0;
}

/// @brief クリップが変化していれば差し替え、再生位置をリセットする
void SetClip(SpriteAnimation& anim, const s3d::String& clip) {
  if (clip != anim.currentClip) {
    anim.currentClip = clip;
    anim.elapsed = 0.0;
  }
}

/// @brief クリップ名が同じでも再生位置を先頭へ強制的に戻す
void RestartClip(SpriteAnimation& anim, const s3d::String& clip) {
  anim.currentClip = clip;
  anim.elapsed = 0.0;
}

/// @brief 近接攻撃判定エンティティ（光の珠）を生成する
///
/// Component/Attack.hpp の Attack（ダメージ用）を付与する。
/// 生成時点では構え中につき珠は体の近くに静止した位置に置く。
/// Collider は珠エンティティ自身の原点からのオフセット 0 で固定し、
/// 珠の現在位置は UpdateMeleeHitbox が更新する LocalOffset のみが担う。
/// @param radius 攻撃カプセルの半径（兼 CircleDrawable の表示半径）
entt::entity SpawnMeleeHitbox(entt::registry& registry, entt::entity owner,
                              const WorldPos& pos, const PlayerConfig& cfg,
                              double radius) {
  const auto hitbox = registry.create();
  registry.emplace<WorldPos>(hitbox, pos);
  registry.emplace<LocalOffset>(hitbox, LocalOffset{});
  Hierarchy::Attach(registry, owner, hitbox);
  registry.emplace<Collider>(hitbox, Collider{.segmentStart = Vec3::Zero(),
                                              .segmentEnd = Vec3::Zero(),
                                              .radius = radius});
  registry.emplace<Attack>(hitbox, Attack{.damage = cfg.melee.damage,
                                          .hitstopSec = KMeleeHitstopSec});
  registry.emplace<Drawable>(
      hitbox, CircleDrawable{.radius = radius, .color = KMeleeOrbColor});
  return hitbox;
}

/// @brief 攻撃フレーム内の珠の前方オフセット（プレイヤー相対）を返す
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
s3d::Vec3 MeleeOrbOffset(double progress, bool facingRight,
                         const MeleeConfig& cfg) {
  const double sign = facingRight ? 1.0 : -1.0;
  const double eased = s3d::EaseOutQuad(std::clamp(progress, 0.0, 1.0));
  return Vec3{sign * cfg.reach * eased, cfg.capMidH, 0.0};
}

/// @brief 2段目の攻撃フレーム内の珠オフセット（斬り上げ軌道）を返す
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
s3d::Vec3 MeleeSlashOffset(double progress, bool facingRight,
                           const MeleeConfig& cfg) {
  const double sign = facingRight ? 1.0 : -1.0;
  const double eased = s3d::EaseOutQuad(std::clamp(progress, 0.0, 1.0));
  const double horizontal = sign * cfg.reach * eased;
  const double vertical = cfg.capMidH + (eased - 0.5) * cfg.slashRiseHeight;
  return Vec3{horizontal, vertical, 0.0};
}

/// @brief Melee1 へ移行する（攻撃クリップの設定）
Melee1 MakeMelee(SpriteAnimation& anim, entt::entity hitboxEntity) {
  SetClip(anim, U"melee_1");
  return Melee1{.elapsed = 0.0, .hitboxEntity = hitboxEntity};
}

/// @brief Melee1 から Melee2 へ移行する（攻撃クリップを先頭から再生）
Melee2 MakeMelee2(SpriteAnimation& anim) {
  RestartClip(anim, U"melee_1");
  return Melee2{};
}

/// @brief Melee2 から Melee3 へ移行する（攻撃クリップを先頭から再生）
Melee3 MakeMelee3(SpriteAnimation& anim) {
  RestartClip(anim, U"melee_1");
  return Melee3{};
}

/// @brief 攻撃判定の発生区間に応じてヒットボックスを生成・更新・破棄する
/// @param radius 攻撃カプセルの半径（兼 CircleDrawable の表示半径）
/// @param offsetFn 攻撃フレーム内の進行度から珠のオフセットを算出する関数
void UpdateMeleeHitbox(entt::registry& registry, entt::entity owner,
                       double elapsed, double activeStart, double activeEnd,
                       bool facingRight, const PlayerConfig& cfg,
                       entt::entity& hitboxEntity, double radius,
                       s3d::Vec3 (*offsetFn)(double, bool,
                                             const MeleeConfig&)) {
  const auto& melee = cfg.melee;

  // 攻撃フレーム中（未生成ならここで生成する）：珠を前方へ EaseOut
  // 補間で移動させる
  if (elapsed >= activeStart && elapsed < activeEnd) {
    if (hitboxEntity == entt::null) {
      const auto& pos = registry.get<WorldPos>(owner);
      hitboxEntity = SpawnMeleeHitbox(registry, owner, pos, cfg, radius);
    }

    const double progress = (elapsed - activeStart) / (activeEnd - activeStart);
    const auto offset = offsetFn(progress, facingRight, melee);

    auto& localOffset = registry.get<LocalOffset>(hitboxEntity);
    localOffset.value = WorldPos{.w = offset.x, .h = offset.y, .d = offset.z};
  }

  // 後隙以降：ヒットボックスが残っていれば破棄する
  if (elapsed >= activeEnd && hitboxEntity != entt::null) {
    Hierarchy::DestroyWithChildren(registry, hitboxEntity);
    hitboxEntity = entt::null;
  }
}

/// @brief 遠距離攻撃の弾エンティティを生成する
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

/// @brief Ranged へ移行する（遠距離攻撃クリップの設定と timer の算出）
Ranged MakeRanged(const AnimationData& playerData, SpriteAnimation& anim) {
  SetClip(anim, U"ranged_attack");
  return Ranged{.timer = GetClipDuration(playerData, U"ranged_attack")};
}

/// @brief Dash へ移行する（スタミナ消費、クリップの設定）
///
/// 専用のダッシュ用クリップは未用意のため、移動主体の動きという特性が近い
/// "move" クリップを暫定的に流用する。
Dash MakeDash(entt::registry& registry, entt::entity entity,
              const PlayerConfig& cfg, SpriteAnimation& anim) {
  auto& stamina = registry.get<Stamina>(entity);
  stamina.current = std::max(0, stamina.current - cfg.dash.staminaCost);
  SetClip(anim, U"move");
  return Dash{};
}

}  // namespace

std::optional<Motion> Tick(Neutral& /*state*/, entt::registry& registry,
                           entt::entity entity, const FrameData& frameData) {
  const auto& input = frameData.input;
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& playerData =
      registry.ctx().get<AnimationDataRegistry>().at(U"player");
  const auto& pos = registry.get<WorldPos>(entity);
  auto& vel = registry.get<Velocity>(entity);
  auto& anim = registry.get<SpriteAnimation>(entity);

  // 移動・ジャンプ・向き
  vel.w = input.moveAxis.x * cfg.speed;
  vel.d = input.moveAxis.y * cfg.speed;

  if (input.jumpDown && pos.isOnGround()) {
    vel.h = cfg.jumpSpeed;
  }

  if (vel.w > 0.0) {
    anim.facingRight = true;
  } else if (vel.w < 0.0) {
    anim.facingRight = false;
  }

  // Melee/Ranged への入場（接地中のみ）
  if (pos.isOnGround()) {
    if (input.attackDown) {
      // 直前で設定した横方向速度を打ち消す（持ち越すと1フレーム分滑る）
      vel.w = 0.0;
      vel.d = 0.0;
      // ヒットボックス（光の珠）は攻撃フレーム開始時に Melee1::Tick が生成する
      return MakeMelee(anim, entt::null);
    }
    if (input.rangedAttackDown) {
      vel.w = 0.0;
      vel.d = 0.0;
      SpawnProjectile(registry, pos, anim.facingRight, cfg);
      return MakeRanged(playerData, anim);
    }
    if (input.dashDown &&
        registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
      return MakeDash(registry, entity, cfg, anim);
    }
  }

  // ロコモーションクリップ（idle/move/jump）
  // pos は前フレームまでの値のため、ジャンプ入力で vel.h を設定した
  // 直後のフレームでは isOnGround() がまだ true のままになる。
  // vel.h > 0.0（ジャンプによる上昇）の場合のみ判定に加えることで、
  // 重力による接地中の微小な負の vel.h には反応せず、
  // 入力と同フレームで jump クリップに切り替える。
  if (!pos.isOnGround() || vel.h > 0.0) {
    SetClip(anim, U"jump");
  } else if (vel.w != 0.0 || vel.d != 0.0) {
    SetClip(anim, U"move");
  } else {
    SetClip(anim, U"idle");
  }

  return std::nullopt;
}

std::optional<Motion> Tick(Melee1& state, entt::registry& registry,
                           entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& melee = cfg.melee;
  const auto& input = frameData.input;
  auto& anim = registry.get<SpriteAnimation>(entity);

  const double activeStart = melee.windupSec;
  const double activeEnd = melee.windupSec + melee.activeSec;
  const double recoveryEnd = activeEnd + melee.recoverySec;

  state.elapsed += frameData.dt;

  UpdateMeleeHitbox(registry, entity, state.elapsed, activeStart, activeEnd,
                    anim.facingRight, cfg, state.hitboxEntity, melee.radius,
                    MeleeOrbOffset);

  // windup・active中の攻撃入力は次段への遷移を予約するのみ
  if (state.elapsed < activeEnd && input.attackDown) {
    state.comboQueued = true;
  }

  // 後隙に入った時点で予約済み、または後隙中の新規入力があれば即座に次段へ
  if (state.elapsed >= activeEnd && (state.comboQueued || input.attackDown)) {
    return MakeMelee2(anim);
  }

  // 後隙中のダッシュ入力でダッシュへキャンセル（ST不足時は無視）
  if (state.elapsed >= activeEnd && input.dashDown &&
      registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
    return MakeDash(registry, entity, cfg, anim);
  }

  if (state.elapsed >= recoveryEnd) {
    return Neutral{};
  }

  return std::nullopt;
}

std::optional<Motion> Tick(Melee2& state, entt::registry& registry,
                           entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& melee = cfg.melee;
  const auto& input = frameData.input;
  auto& anim = registry.get<SpriteAnimation>(entity);

  const double activeStart = melee.windupSec;
  const double activeEnd = melee.windupSec + melee.active2Sec;
  const double recoveryEnd = activeEnd + melee.recoverySec;

  state.elapsed += frameData.dt;

  UpdateMeleeHitbox(registry, entity, state.elapsed, activeStart, activeEnd,
                    anim.facingRight, cfg, state.hitboxEntity, melee.radius,
                    MeleeSlashOffset);

  // windup・active中の攻撃入力は次段への遷移を予約するのみ
  if (state.elapsed < activeEnd && input.attackDown) {
    state.comboQueued = true;
  }

  // 後隙に入った時点で予約済み、または後隙中の新規入力があれば即座に次段へ
  if (state.elapsed >= activeEnd && (state.comboQueued || input.attackDown)) {
    return MakeMelee3(anim);
  }

  // 後隙中のダッシュ入力でダッシュへキャンセル（ST不足時は無視）
  if (state.elapsed >= activeEnd && input.dashDown &&
      registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
    return MakeDash(registry, entity, cfg, anim);
  }

  if (state.elapsed >= recoveryEnd) {
    return Neutral{};
  }

  return std::nullopt;
}

std::optional<Motion> Tick(Melee3& state, entt::registry& registry,
                           entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& melee = cfg.melee;
  const auto& anim = registry.get<SpriteAnimation>(entity);

  const double activeStart = melee.windup3Sec;
  const double activeEnd = melee.windup3Sec + melee.active3Sec;
  const double recoveryEnd = activeEnd + melee.recovery3Sec;

  state.elapsed += frameData.dt;

  UpdateMeleeHitbox(registry, entity, state.elapsed, activeStart, activeEnd,
                    anim.facingRight, cfg, state.hitboxEntity, melee.radius3,
                    MeleeOrbOffset);

  // 締め技のためコンボ継続なし、後隙満了で Neutral へ戻るのみ
  if (state.elapsed >= recoveryEnd) {
    return Neutral{};
  }

  return std::nullopt;
}

std::optional<Motion> Tick(Ranged& state, entt::registry& registry,
                           entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    return Neutral{};
  }

  return std::nullopt;
}

std::optional<Motion> Tick(Dash& state, entt::registry& registry,
                           entt::entity entity, const FrameData& frameData) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& dash = cfg.dash;
  const auto& input = frameData.input;
  auto& vel = registry.get<Velocity>(entity);
  auto& anim = registry.get<SpriteAnimation>(entity);

  const double dashStart = dash.windupSec;
  const double dashEnd = dash.windupSec + dash.dashSec;
  const double recoveryAEnd = dashEnd + dash.recoveryASec;
  const double recoveryBEnd = recoveryAEnd + dash.recoveryBSec;

  state.elapsed += frameData.dt;

  // 構え・ダッシュ中（後隙入り前）は無敵、後隙入りで除去する
  if (state.elapsed < dashEnd) {
    registry.emplace_or_replace<Invincible>(entity);
  } else {
    registry.remove<Invincible>(entity);
  }

  // ダッシュ移動中：フリー方向、無方向なら facingRight から前方
  if (state.elapsed >= dashStart && state.elapsed < dashEnd) {
    if (!input.moveAxis.isZero()) {
      const Vec2 dir = input.moveAxis.normalized();
      vel.w = dir.x * dash.speed;
      vel.d = dir.y * dash.speed;
    } else {
      vel.w = (anim.facingRight ? 1.0 : -1.0) * dash.speed;
      vel.d = 0.0;
    }
  } else {
    vel.w = 0.0;
    vel.d = 0.0;
  }

  // 後隙B中のダッシュ入力はダッシュ攻撃への遷移を予約するのみ
  // （遷移先の実装は #164）
  if (state.elapsed >= recoveryAEnd && input.dashDown) {
    state.dashAttackQueued = true;
  }

  if (state.elapsed >= recoveryBEnd) {
    return Neutral{};
  }

  return std::nullopt;
}

}  // namespace PlayerMotion
