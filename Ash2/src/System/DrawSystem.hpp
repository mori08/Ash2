#pragma once
#include <compare>
#include <entt/entt.hpp>

#include "Component/WorldPos.hpp"

/// @brief 描画順の比較に使うキー
struct DrawOrderKey {
  /// 奥行き（`WorldPos::d`）
  double d = 0.0;
  /// 同じ `d` の順序を確定させるタイブレーカ
  entt::entity entity = entt::null;
};

/// @brief 描画順の比較関数（奥から手前の順）
/// @return a が b より先に描かれる場合 true。d は降順、d が等しい場合は entity
/// の昇順
[[nodiscard]] inline bool DrawOrderLess(
    const DrawOrderKey& a, const DrawOrderKey& b
) {
  // 奥（d が大きい方）が先
  if (const auto order = b.d <=> a.d; order != 0) {
    return order < 0;
  }
  return a.entity < b.entity;
}

/// @brief スプライト描画システム
class DrawSystem {
 public:
  /// @brief WorldPos + Drawable を持つエンティティを奥から順に描画する
  static void Draw(const entt::registry& registry);
};
