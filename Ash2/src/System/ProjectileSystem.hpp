#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

struct FrameData;

/// @brief 飛翔体（弾）の移動・消滅を管理するシステム
class ProjectileSystem {
 public:
  /// @brief Projectile
  /// を持つエンティティを移動させ、消滅条件を満たしたものを破棄する
  ///
  /// `Velocity` に従って `WorldPos` を更新したのち、以下のいずれかを満たす
  /// エンティティを `registry.destroy()` する。
  /// - 着弾: `Attack.hitTargets` が空でなくなった（`HitSystem`
  /// がヒットを記録した）
  /// - 画面外: `WorldPos` を画面座標に変換した結果が `Scene::Rect()` の範囲外
  ///
  /// 着弾判定が正しく行われるよう、`HitSystem::Update` の後に呼び出すこと。
  static void Update(entt::registry& registry, double dt);
};
