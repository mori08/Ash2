#include <Siv3D.hpp>

#include "System/PlayerMotion/Helper.hpp"

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"

namespace PlayerMotion {

namespace {

constexpr ColorF KMeleeOrbColor = {1.0, 0.9, 0.5};
/// 暫定のヒットストップ時間（秒）。本格的な数値調整は #132/#134 で行う
constexpr double KMeleeHitstopSec = 0.05;

/// @brief 攻撃判定エンティティの半径・ダメージ量をまとめた仕様
// SpawnAttackHitbox の引数で double の radius と int32 の damage が隣接すると
// 呼び出し側で取り違えやすいため、1つの構造体にまとめて渡す
// （bugprone-easily-swappable-parameters 対策）。
struct HitboxSpec {
  /// 攻撃カプセルの半径（兼 CircleDrawable の表示半径）
  double radius = 0.0;
  /// 与えるダメージ量
  int32 damage = 0;
  /// 被弾側リアクションの強さ
  ReactionLevel reaction = ReactionLevel::None;
};

/// @brief 攻撃判定エンティティ（光の珠）を生成する
///
/// Component/Attack.hpp の Attack（ダメージ用）を spec.damage
/// で確定させて付与する（生成後の上書きは行わない）。生成時点では構え中につき
/// 珠は体の近くに静止した位置に置く。Collider は珠エンティティ自身の原点からの
/// オフセット 0 で固定し、珠の現在位置は UpdateAttackHitbox が更新する
/// LocalOffset のみが担う。
entt::entity SpawnAttackHitbox(entt::registry& registry, entt::entity owner,
                               const WorldPos& pos, const HitboxSpec& spec) {
  const auto hitbox = registry.create();
  registry.emplace<WorldPos>(hitbox, pos);
  registry.emplace<LocalOffset>(hitbox, LocalOffset{});
  Hierarchy::Attach(registry, owner, hitbox);
  registry.emplace<Collider>(hitbox, Collider{.segmentStart = Vec3::Zero(),
                                              .segmentEnd = Vec3::Zero(),
                                              .radius = spec.radius});
  registry.emplace<Attack>(hitbox, Attack{.damage = spec.damage,
                                          .hitstopSec = KMeleeHitstopSec,
                                          .reaction = spec.reaction});
  registry.emplace<Drawable>(
      hitbox, CircleDrawable{.radius = spec.radius, .color = KMeleeOrbColor});
  return hitbox;
}

}  // namespace

void SetClip(SpriteAnimation& anim, const String& clip) {
  if (clip != anim.currentClip) {
    anim.currentClip = clip;
    anim.elapsed = 0.0;
  }
}

void StopHorizontalMovement(entt::registry& registry, entt::entity entity) {
  auto& vel = registry.get<Velocity>(entity);
  vel.w = 0.0;
  vel.d = 0.0;
}

void UpdateAttackHitbox(entt::registry& registry, entt::entity owner,
                        double elapsed, const MotionTimeline& timeline,
                        double radius, int32 damage, ReactionLevel reaction,
                        entt::entity& hitboxEntity,
                        const std::function<Vec3(double)>& offsetFn) {
  // 攻撃フレーム中（未生成ならここで生成する）：珠を前方へ EaseOut
  // 補間で移動させる
  if (timeline.isActive(elapsed)) {
    if (hitboxEntity == entt::null) {
      const auto& pos = registry.get<WorldPos>(owner);
      hitboxEntity = SpawnAttackHitbox(
          registry, owner, pos,
          HitboxSpec{.radius = radius, .damage = damage, .reaction = reaction});
    }

    const auto offset = offsetFn(timeline.activeProgress(elapsed));

    auto& localOffset = registry.get<LocalOffset>(hitboxEntity);
    localOffset.value = WorldPos{.w = offset.x, .h = offset.y, .d = offset.z};
  }

  // 後隙以降：ヒットボックスが残っていれば破棄する
  if (elapsed >= timeline.activeEnd() && hitboxEntity != entt::null) {
    Hierarchy::DestroyWithChildren(registry, hitboxEntity);
    hitboxEntity = entt::null;
  }
}

}  // namespace PlayerMotion
