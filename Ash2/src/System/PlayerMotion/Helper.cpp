#include <Siv3D.hpp>

#include "System/PlayerMotion/Helper.hpp"

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/FadeOut.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"

namespace PlayerMotion {

namespace {

constexpr ColorF KMeleeOrbColor = {1.0, 0.9, 0.5};

/// @brief 攻撃判定エンティティ（光の珠）を生成する
///
/// Component/Attack.hpp の Attack（ダメージ用）を spec.damage
/// で確定させて付与する（生成後の上書きは行わない）。生成時点では構え中につき
/// 珠は体の近くに静止した位置に置く。Collider は珠エンティティ自身の原点からの
/// オフセット 0 で固定し、珠の現在位置は UpdateAttackHitbox が更新する
/// LocalOffset のみが担う。
entt::entity SpawnAttackHitbox(
    entt::registry& registry, entt::entity owner, const WorldPos& pos,
    const HitboxSpec& spec
) {
  const auto hitbox = registry.create();
  registry.emplace<WorldPos>(hitbox, pos);
  registry.emplace<LocalOffset>(hitbox, LocalOffset{});
  Hierarchy::Attach(registry, owner, hitbox);
  registry.emplace<Collider>(
      hitbox,
      Collider{
          .segmentStart = Vec3::Zero(),
          .segmentEnd = Vec3::Zero(),
          .radius = spec.radius
      }
  );
  registry.emplace<Attack>(
      hitbox,
      Attack{
          .damage = spec.damage,
          .hitstopSec = spec.hitstopSec,
          .reaction = spec.reaction
      }
  );
  registry.emplace<Drawable>(hitbox, CircleDrawable{.radius = spec.radius});
  registry.emplace<DrawColor>(hitbox, DrawColor{.color = KMeleeOrbColor});
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

entt::entity ReleaseAttackHitbox(
    entt::registry& registry, entt::entity hitboxEntity, double fadeSec
) {
  Hierarchy::Detach(registry, hitboxEntity);
  registry.remove<Attack>(hitboxEntity);
  if (fadeSec <= 0.0) {
    registry.destroy(hitboxEntity);
  } else {
    registry.emplace_or_replace<FadeOut>(
        hitboxEntity, FadeOut{.duration = fadeSec, .remaining = fadeSec}
    );
  }
  return entt::null;
}

void UpdateAttackHitbox(
    entt::registry& registry, entt::entity owner, double elapsed,
    const MotionTimeline& timeline, const HitboxSpec& spec,
    entt::entity& hitboxEntity, const std::function<Vec3(double)>& offsetFn
) {
  // 攻撃フレーム中（未生成ならここで生成する）：珠を前方へ EaseOut
  // 補間で移動させる
  if (timeline.isActive(elapsed)) {
    if (hitboxEntity == entt::null) {
      const auto& pos = registry.get<WorldPos>(owner);
      hitboxEntity = SpawnAttackHitbox(registry, owner, pos, spec);
    }

    const auto offset = offsetFn(timeline.activeProgress(elapsed));

    auto& localOffset = registry.get<LocalOffset>(hitboxEntity);
    localOffset.value = WorldPos{.w = offset.x, .h = offset.y, .d = offset.z};
  }

  // 後隙以降：ヒットボックスが残っていれば解放する
  if (elapsed >= timeline.activeEnd() && hitboxEntity != entt::null) {
    hitboxEntity = ReleaseAttackHitbox(registry, hitboxEntity, spec.fadeSec);
  }
}

}  // namespace PlayerMotion
