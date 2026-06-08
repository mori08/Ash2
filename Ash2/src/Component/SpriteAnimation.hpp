#pragma once
#include <Siv3D.hpp>

/// @brief スプライトアニメーションコンポーネント（per-entity、軽量）
struct SpriteAnimation {
  /// AnimationDataRegistry のキー（アニメーション設定ファイル名由来）
  s3d::String dataKey;
  s3d::String currentClip;
  /// 現クリップ内の位相（秒）。[0, count/speed) の範囲にラップされる
  double elapsed = 0.0;
  /// スプライトは左向きがデフォルト。true のとき AnimationSystem が反転描画する
  bool facingRight = false;
};
