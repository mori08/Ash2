#pragma once
#include <entt/entt.hpp>

#include "Component/WorldPos.hpp"

/// @brief 描画順の比較関数（奥から手前の順）
/// @return a が b より奥にある場合 true
inline bool DrawOrderLess(const WorldPos& a, const WorldPos& b) {
  return a.d > b.d;
}

/// @brief スプライト描画システム
class DrawSystem {
 public:
  /// @brief WorldPos + Drawable を持つエンティティを奥から順に描画する
  static void Draw(const entt::registry& registry);
};
