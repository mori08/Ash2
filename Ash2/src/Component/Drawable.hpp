#pragma once
#include <Siv3D.hpp>

#include <variant>

/// @brief 矩形スプライト描画データ
struct RectDrawable {
  /// 描画サイズ（幅・高さ）
  SizeF size;
  /// 描画色
  ColorF color;
};

/// @brief 描画コンポーネント（描画形状の variant）
using Drawable = std::variant<RectDrawable>;
