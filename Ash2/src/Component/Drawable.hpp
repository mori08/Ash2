#pragma once
#include <Siv3D.hpp>

#include <variant>

/// @brief 枠線スタイル
struct BorderStyle {
  /// 枠線の色
  ColorF color;
  /// 枠線の太さ
  double thickness;
};

/// @brief 矩形描画データ
struct RectDrawable {
  /// 描画サイズ（幅・高さ）
  SizeF size;
  /// 描画色
  ColorF color;
  /// 枠線（none = 枠線なし）
  Optional<BorderStyle> border;
};

/// @brief 円描画データ
struct CircleDrawable {
  /// 半径
  double radius;
  /// 描画色
  ColorF color;
  /// 枠線（none = 枠線なし）
  Optional<BorderStyle> border;
};

/// @brief 扇形描画データ
struct PieDrawable {
  /// 半径
  double radius;
  /// 開始角度（ラジアン、12時方向から時計回り）
  double startAngle;
  /// 角度（ラジアン）
  double angle;
  /// 描画色
  ColorF color;
  /// 枠線（none = 枠線なし）
  Optional<BorderStyle> border;
};

/// @brief テクスチャ描画データ
struct TextureDrawable {
  /// 描画テクスチャ領域
  TextureRegion region;
  /// 描画オフセット（中心座標からのずれ、アンカー調整用）
  Vec2 drawOffset{0, 0};
};

/// @brief 描画コンポーネント（描画形状の variant）
using Drawable =
    std::variant<RectDrawable, CircleDrawable, PieDrawable, TextureDrawable>;
