#pragma once
#include <Siv3D.hpp>

#include <variant>

/// @brief WorldPos を描画形状内のどの点に合わせるか
enum class DrawAnchor : uint8 {
  /// 形状の中心
  Center,
  /// 形状の下端中央
  BottomCenter,
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

/// @brief 描画コンポーネント（描画形状の variant）
using Drawable = std::variant<RectDrawable, CircleDrawable, TextureDrawable>;
