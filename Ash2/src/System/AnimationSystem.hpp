#pragma once
#include <entt/entt.hpp>

/// @brief スプライトアニメーション更新システム
class AnimationSystem {
 public:
  /// @brief SpriteAnimation を持つエンティティのフレームを更新する
  /// @param registry ECS レジストリ
  /// @param dt フレーム経過時間（秒）
  static void Update(entt::registry& registry, double dt);
};
