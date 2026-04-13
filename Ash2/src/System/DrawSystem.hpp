#pragma once
#include <ThirdParty/entt/entt.hpp>

/// @brief スプライト描画システム
class DrawSystem {
 public:
  /// @brief WorldPos + Drawable を持つエンティティを奥から順に描画する
  /// @param registry ECS レジストリ
  static void Draw(const entt::registry& registry);
};
