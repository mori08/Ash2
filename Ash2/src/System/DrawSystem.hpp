#pragma once
#include <Siv3D.hpp>

#include <compare>
#include <entt/entt.hpp>

/// @brief 描画順の比較に使うキー
struct DrawOrderKey {
  /// 前後関係の帯（`DrawPos::layer`）
  int32 layer = 0;
  /// 同じ `layer` 内の奥行き（`DrawPos::depth`）
  double depth = 0.0;
  /// 同じ `layer`・`depth` の順序を確定させるタイブレーカ
  entt::entity entity = entt::null;
};

/// @brief 描画順の比較関数（奥から手前の順）
/// @return a が b より先に描かれる場合 true。layer は昇順、同じ layer では
/// depth の降順、depth も等しい場合は entity の昇順
[[nodiscard]] inline bool DrawOrderLess(
    const DrawOrderKey& a, const DrawOrderKey& b
) {
  if (const auto order = a.layer <=> b.layer; order != 0) {
    return order < 0;
  }
  // 奥（depth が大きい方）が先
  if (const auto order = b.depth <=> a.depth; order != 0) {
    return order < 0;
  }
  return a.entity < b.entity;
}

/// @brief 描画システム
///
/// ワールドも画面固定のものも `DrawPos` だけを見て、1系統で描く。
class DrawSystem {
 public:
  /// @brief DrawPos + Drawable を持つエンティティを (layer, depth)
  /// の順に描画する
  static void Draw(const entt::registry& registry);
};
