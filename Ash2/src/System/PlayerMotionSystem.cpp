#include <Siv3D.hpp>

#include "System/PlayerMotionSystem.hpp"

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/AnimationData.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"

namespace PlayerMotion {

namespace {

constexpr s3d::ColorF KBulletColor = {0.9, 0.9, 0.3};

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

/// @brief 近距離攻撃のヒットボックスエンティティを生成する
///
/// Component/Attack.hpp の Attack（ダメージ用）を付与する。
/// PlayerMotion::Melee の "attack" とは別概念。
entt::entity SpawnMeleeHitbox(entt::registry& registry, entt::entity owner,
                              const WorldPos& pos, bool facingRight,
                              const PlayerConfig& cfg) {
  const double sign = facingRight ? 1.0 : -1.0;
  const auto hitbox = registry.create();
  registry.emplace<WorldPos>(hitbox, pos);
  registry.emplace<LocalOffset>(hitbox, LocalOffset{});
  Hierarchy::Attach(registry, owner, hitbox);
  registry.emplace<Collider>(
      hitbox, Collider{.segmentStart = Vec3{0.0, cfg.melee.capMidH, 0.0},
                       .segmentEnd =
                           Vec3{sign * cfg.melee.reach, cfg.melee.capMidH, 0.0},
                       .radius = cfg.melee.radius});
  registry.emplace<Attack>(hitbox, Attack{.damage = cfg.melee.damage});
  return hitbox;
}

/// @brief Melee へ移行する（攻撃クリップの設定と timer の算出）
Melee MakeMelee(const AnimationData& playerData, SpriteAnimation& anim,
                entt::entity hitboxEntity) {
  SetClip(anim, U"attack");
  return Melee{.timer = GetClipDuration(playerData, U"attack"),
               .hitboxEntity = hitboxEntity};
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
      const auto hitbox =
          SpawnMeleeHitbox(registry, entity, pos, anim.facingRight, cfg);
      return MakeMelee(playerData, anim, hitbox);
    }
    if (input.rangedAttackDown) {
      vel.w = 0.0;
      vel.d = 0.0;
      SpawnProjectile(registry, pos, anim.facingRight, cfg);
      return MakeRanged(playerData, anim);
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

std::optional<Motion> Tick(Melee& state, entt::registry& registry,
                           entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    if (state.hitboxEntity != entt::null) {
      Hierarchy::DestroyWithChildren(registry, state.hitboxEntity);
    }
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

}  // namespace PlayerMotion
