#pragma once
#include <Siv3D.hpp>

#include <variant>

/// @brief 枠線スタイル
struct BorderStyle {
  ColorF color;
  double thickness;
};

/// @brief 矩形描画データ
struct RectDrawable {
  /// 描画サイズ（幅・高さ）
  SizeF size;
  ColorF color;
  /// none = 枠線なし
  Optional<BorderStyle> border;
};

/// @brief 円描画データ
struct CircleDrawable {
  double radius;
  ColorF color;
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
  ColorF color;
  /// none = 枠線なし
  Optional<BorderStyle> border;
};

/// @brief テクスチャ描画データ
struct TextureDrawable {
  TextureRegion region;
  /// 中心座標からのずれ（アンカー調整用）
  Vec2 drawOffset{0, 0};
};

/// @brief 描画コンポーネント（描画形状の variant）
using Drawable =
    std::variant<RectDrawable, CircleDrawable, PieDrawable, TextureDrawable>;
