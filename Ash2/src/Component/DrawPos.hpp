#pragma once
#include <Siv3D.hpp>

/// @brief 描画の前後関係の帯（`DrawPos::layer` に使う。大きいほど手前）
namespace DrawLayer {
/// キャラクター・弾・攻撃の光
inline constexpr int32 kWorld = 0;
/// ワールドの上に重ねるもの（レティクル）
inline constexpr int32 kWorldOverlay = 100;
/// ゲージ・メニュー・ロゴ・CLEAR 文字
inline constexpr int32 kHud = 200;
}  // namespace DrawLayer

/// @brief 描画専用の座標（画面座標 px）
///
/// DrawSystem はこの座標だけを見て描く。WorldPos を持つエンティティでは
/// ProjectionSystem が `pos` と `depth` を毎フレーム上書きする。WorldPos を
/// 持たないエンティティ（HUD・メニュー）は生成時の値がそのまま使われる。
struct DrawPos {
  /// 画面座標（px）
  Vec2 pos;
  /// 前後関係の帯（大きいほど手前）。`DrawLayer` の定数を使う
  int32 layer = DrawLayer::kWorld;
  /// 同じ `layer` 内の奥行き（大きいほど奥で、先に描く）。同値は entity 昇順
  double depth = 0.0;
};
