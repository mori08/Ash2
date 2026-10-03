#pragma once
#include <Siv3D.hpp>

/// @brief 画面固定の描画位置（px）
///
/// WorldPos とは異なり、HudSystem が画面座標へ直接描く対象であることを示す。
struct ScreenPos {
  Vec2 pos;
  /// HudSystem が描く前後関係（大きいほど手前）。同値は entity 昇順
  int32 layer = 0;
};
