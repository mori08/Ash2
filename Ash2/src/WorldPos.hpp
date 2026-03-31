#pragma once
#include <Siv3D.hpp>

// ワールド座標
// w: 横位置
// h: 高さ（地面 = 0、上方向が正）
// d: 奥行き（大きいほど奥 = 画面上方）
struct WorldPos {
  double w = 0.0;
  double h = 0.0;
  double d = 0.0;

  Vec2 ToScreen() const { return {w, -(d + h)}; }
};

// 描画順比較：奥（d 大）から手前（d 小）の順
inline bool DrawOrderLess(const WorldPos& a, const WorldPos& b) {
  return a.d > b.d;
}
