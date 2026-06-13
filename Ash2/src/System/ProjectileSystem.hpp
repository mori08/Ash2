#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

struct FrameData;

/// @brief 飛翔体（弾）の消滅を管理するシステム
class ProjectileSystem {
 public:
  /// @brief Projectile を持つエンティティのうち消滅条件を満たしたものを破棄する
  ///
  /// 以下のいずれかを満たすエンティティを `registry.destroy()` する。
  /// - 着弾: `Attack.hitTargets` が空でなくなった（`HitSystem`
  /// がヒットを記録した）
  /// - 画面外: `WorldPos` を画面座標に変換した結果が `Scene::Rect()` の範囲外
  ///
  /// 位置更新は `MovementSystem` が担うため、`MovementSystem::Update`
  /// および `HitSystem::Update` の後に呼び出すこと。
  static void Update(entt::registry& registry);
};
