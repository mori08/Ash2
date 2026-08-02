#pragma once
#include <Siv3D.hpp>

#include <variant>

/// @brief 枠線スタイル
struct BorderStyle {
  ColorF color;
  double thickness;
};

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
  /// none = 枠線なし
  Optional<BorderStyle> border;
  DrawAnchor anchor = DrawAnchor::Center;
};

/// @brief 円描画データ
struct CircleDrawable {
  double radius;
  /// none = 枠線なし
  Optional<BorderStyle> border;
};

/// @brief 扇形描画データ
struct PieDrawable {
  double radius;
  /// ラジアン、12時方向から時計回り
  double startAngle;
  /// ラジアン
  double angle;
  /// none = 枠線なし
  Optional<BorderStyle> border;
};

/// @brief テクスチャ描画データ
struct TextureDrawable {
  TextureRegion region;
  /// anchor が示す位置からのずれ
  Vec2 drawOffset{0, 0};
  DrawAnchor anchor = DrawAnchor::Center;
};

/// @brief 描画コンポーネント（描画形状の variant）
using Drawable =
    std::variant<RectDrawable, CircleDrawable, PieDrawable, TextureDrawable>;
