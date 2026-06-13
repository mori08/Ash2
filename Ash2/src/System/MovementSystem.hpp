#pragma once
#include <entt/entt.hpp>

/// @brief Velocity に従って WorldPos を更新する汎用システム
class MovementSystem {
 public:
  /// @brief WorldPos + Velocity を持つ全エンティティの位置を更新する
  ///
  /// `pos.w/h/d += vel.w/h/d * dt` を適用する。
  static void Update(entt::registry& registry, double dt);
};
