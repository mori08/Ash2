#pragma once
#include <Siv3D.hpp>

#include <variant>

/// @brief WorldPos / ScreenPos を描画範囲のどの点に合わせるか
enum class DrawAnchor : uint8 {
  /// 描画範囲の中心
  Center,
  /// 描画範囲の下端中央
  BottomCenter,
  /// 描画範囲の左上
  TopLeft,
};

/// @brief 矩形描画データ
struct RectDrawable {
  /// 描画サイズ（幅・高さ）
  SizeF size;
  DrawAnchor anchor = DrawAnchor::Center;
};

/// @brief 円描画データ
struct CircleDrawable {
  double radius;
};

/// @brief テクスチャ描画データ
struct TextureDrawable {
  TextureRegion region;
  /// anchor が示す位置からのずれ
  Vec2 drawOffset{0, 0};
  DrawAnchor anchor = DrawAnchor::Center;
};

/// @brief 文字描画データ
struct TextDrawable {
  /// 描画する文字列
  String text;
  Font font;
  DrawAnchor anchor = DrawAnchor::Center;
};

/// @brief 描画コンポーネント（描画データの variant）
using Drawable =
    std::variant<RectDrawable, CircleDrawable, TextureDrawable, TextDrawable>;
