#include "System/BattleSystem.hpp"

#include <ranges>

#include "Component/FadeOut.hpp"
#include "Component/Projectile.hpp"
#include "FrameData.hpp"
#include "System/AnimationSystem.hpp"
#include "System/AttachmentSystem.hpp"
#include "System/BoundarySystem.hpp"
#include "System/EnemySystem.hpp"
#include "System/FadeOutSystem.hpp"
#include "System/GravitySystem.hpp"
#include "System/HitReactionSystem.hpp"
#include "System/HitSystem.hpp"
#include "System/HitstopSystem.hpp"
#include "System/LockOnSystem.hpp"
#include "System/MotionSystem.hpp"
#include "System/MovementSystem.hpp"
#include "System/ProjectileSystem.hpp"
#include "System/StaminaSystem.hpp"

namespace {

/// @brief Tag を持つエンティティをすべて破棄する
template <typename Tag>
void DestroyAllWith(entt::registry& registry) {
  // 走査中に destroy するとビューの走査が壊れるため、
  // 先に配列へ集めてから破棄する
  const auto toDestroy =
      registry.view<Tag>() | std::ranges::to<Array<entt::entity>>();
  for (const auto entity : toDestroy) {
    registry.destroy(entity);
  }
}

}  // namespace

void BattleSystem::Update(
    entt::registry& registry, const FrameData& frameData
) {
  const double dt = frameData.dt;

  HitstopSystem::Update(registry, dt);
  LockOnSystem::Update(registry, frameData);
  MotionSystem::Update(registry, frameData);
  StaminaSystem::Update(registry, dt);
  MovementSystem::Update(registry, dt);
  GravitySystem::Update(registry, dt);
  BoundarySystem::Update(registry);
  AttachmentSystem::UpdateTransform(registry);

  const auto hits = HitSystem::Update(registry);
  HitReactionSystem::Apply(registry, hits);
  ProjectileSystem::Update(registry);
  EnemySystem::Update(registry);
  FadeOutSystem::Update(registry, dt);

  AnimationSystem::Update(registry, dt);
}

void BattleSystem::UpdateAftermath(
    entt::registry& registry, const FrameData& frameData
) {
  const double dt = frameData.dt;

  HitstopSystem::Update(registry, dt);
  FadeOutSystem::Update(registry, dt);
  AnimationSystem::Update(registry, dt);
}

void BattleSystem::Cleanup(entt::registry& registry) {
  // 弾は独立エンティティ（親を持たない）なので、
  // Projectile タグで検索して個別に破棄する
  DestroyAllWith<Projectile>(registry);

  // フェード中のヒットボックスも所有者から Detach 済みの独立
  // エンティティなので、FadeOut タグで検索して個別に破棄する
  DestroyAllWith<FadeOut>(registry);
}
