#pragma once
#include <entt/entt.hpp>

/// @brief スプライトアニメーション更新システム
class AnimationSystem {
 public:
  /// @brief SpriteAnimation を持つエンティティのフレームを更新する
  static void Update(entt::registry& registry, double dt);
};
