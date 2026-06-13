#include "System/ProjectileSystem.hpp"

#include "Component/Attack.hpp"
#include "Component/Projectile.hpp"
#include "Component/WorldPos.hpp"

void ProjectileSystem::Update(entt::registry& registry) {
  const Vec2 cameraOffset = Scene::Center();
  const RectF screenRect = Scene::Rect();

  s3d::Array<entt::entity> toDestroy;

  auto view = registry.view<Projectile, WorldPos, Attack>();
  for (auto&& [entity, pos, atk] : view.each()) {
    // 着弾: HitSystem がヒットを記録した
    if (!atk.hitTargets.empty()) {
      toDestroy.push_back(entity);
      continue;
    }

    // 画面外: 画面座標に変換した結果が Scene::Rect() の範囲外
    const Vec2 screenPos = cameraOffset + pos.toScreen();
    if (!screenRect.contains(screenPos)) {
      toDestroy.push_back(entity);
    }
  }

  for (const auto entity : toDestroy) {
    registry.destroy(entity);
  }
}
