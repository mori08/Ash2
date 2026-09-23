#include "System/ProjectileSystem.hpp"

#include "Component/Attack.hpp"
#include "Component/Projectile.hpp"
#include "Component/WorldPos.hpp"
#include "Screen.hpp"

namespace {

// 弾が中心のみで判定されるため、描画半径を上回る余裕を持たせてから消す
constexpr double kOffscreenMargin = 32.0;

}  // namespace

void ProjectileSystem::Update(entt::registry& registry) {
  const RectF screenRect = RectF{Scene::Rect()}.stretched(kOffscreenMargin);

  Array<entt::entity> toDestroy;

  auto view = registry.view<const Projectile, WorldPos, Attack>();
  for (auto&& [entity, projectile, pos, atk] : view.each()) {
    // 着弾: HitSystem がヒットを記録した
    if (!atk.hitTargets.empty()) {
      toDestroy.push_back(entity);
      continue;
    }

    // 画面外: 画面座標に変換した結果が kOffscreenMargin ぶん広げた
    // Scene::Rect() の範囲外
    const Vec2 screenPos = WorldToScreen(pos);
    if (!screenRect.contains(screenPos)) {
      toDestroy.push_back(entity);
      continue;
    }

    // 最大射程: 発射位置からの3軸距離が maxRange を超えた
    const Vec3 traveled{
        pos.w - projectile.origin.w, pos.h - projectile.origin.h,
        pos.d - projectile.origin.d
    };
    if (traveled.length() > projectile.maxRange) {
      toDestroy.push_back(entity);
    }
  }

  for (const auto entity : toDestroy) {
    registry.destroy(entity);
  }
}
