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

constexpr ColorF kMeleeOrbColor = {1.0, 0.9, 0.5};

/// @brief 珠エンティティ（判定用・見た目用いずれにも使う）を1つ生成する
///
/// 生成時点では構え中につき体の近くに静止した位置に置く。現在位置は
/// UpdateAttackHitbox/UpdateAttackLights が更新する LocalOffset のみが担う。
/// @param visible true なら Drawable/DrawColor を付与して描画対象にする
entt::entity SpawnOrb(
    entt::registry& registry, entt::entity owner, const WorldPos& pos,
    double radius, bool visible
) {
  const auto orb = registry.create();
  registry.emplace<WorldPos>(orb, pos);
  registry.emplace<LocalOffset>(orb, LocalOffset{});
  Hierarchy::Attach(registry, owner, orb);
  if (visible) {
    registry.emplace<Drawable>(orb, CircleDrawable{.radius = radius});
    registry.emplace<DrawColor>(orb, DrawColor{.color = kMeleeOrbColor});
  }
  return orb;
}

/// @brief 攻撃判定エンティティを生成する
///
/// Component/Attack.hpp の Attack（ダメージ用）を spec.damage
/// で確定させて付与する（生成後の上書きは行わない）。Collider は珠エンティティ
/// 自身の原点からのオフセット 0 で固定し、珠の現在位置は UpdateAttackHitbox
/// が更新する LocalOffset のみが担う。
entt::entity SpawnAttackHitbox(
    entt::registry& registry, entt::entity owner, const WorldPos& pos,
    const HitboxSpec& spec
) {
  const auto hitbox = SpawnOrb(registry, owner, pos, spec.radius, spec.drawOrb);
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
  return hitbox;
}

/// @brief 珠の LocalOffset をワールド相対オフセットへ更新する
void SetOrbOffset(entt::registry& registry, entt::entity orb, Vec3 offset) {
  auto& localOffset = registry.get<LocalOffset>(orb);
  localOffset = LocalOffset{.w = offset.x, .h = offset.y, .d = offset.z};
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

    SetOrbOffset(
        registry, hitboxEntity, offsetFn(timeline.activeProgress(elapsed))
    );
  }

  // 後隙以降：ヒットボックスが残っていれば解放する
  if (elapsed >= timeline.activeEnd() && hitboxEntity != entt::null) {
    hitboxEntity = ReleaseAttackHitbox(registry, hitboxEntity, spec.fadeSec);
  }
}

void UpdateAttackLights(
    entt::registry& registry, entt::entity owner, double elapsed,
    const MotionTimeline& timeline, const LightSpec& spec,
    Array<entt::entity>& lightEntities,
    const std::function<Vec3(double, int32)>& offsetFn
) {
  // 攻撃フレーム中（未生成ならここで生成する）：光を前方へ移動させる
  if (timeline.isActive(elapsed)) {
    if (lightEntities.isEmpty()) {
      const auto& pos = registry.get<WorldPos>(owner);
      for (int32 i = 0; i < spec.count; ++i) {
        lightEntities.push_back(
            SpawnOrb(registry, owner, pos, spec.radius, /*visible=*/true)
        );
      }
    }

    const double progress = timeline.activeProgress(elapsed);
    for (size_t i = 0; i < lightEntities.size(); ++i) {
      SetOrbOffset(
          registry, lightEntities[i], offsetFn(progress, static_cast<int32>(i))
      );
    }
  }

  // 後隙以降：光が残っていれば解放する
  if (elapsed >= timeline.activeEnd() && !lightEntities.isEmpty()) {
    for (const auto light : lightEntities) {
      ReleaseAttackHitbox(registry, light, spec.fadeSec);
    }
    lightEntities.clear();
  }
}

}  // namespace PlayerMotion
