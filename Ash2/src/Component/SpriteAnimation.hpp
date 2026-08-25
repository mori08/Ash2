#pragma once
#include <Siv3D.hpp>

/// @brief スプライトアニメーションコンポーネント（per-entity、軽量）
struct SpriteAnimation {
  /// AnimationDataRegistry のキー（アニメーション設定ファイル名由来）
  String dataKey;
  String currentClip;
  /// 現クリップ内の位相（秒）。loop なら [0, count/speed) にラップされ、
  /// そうでなければ count/speed（上限含む）でクランプされる
  double elapsed = 0.0;
  /// スプライトは左向きがデフォルト。true のとき AnimationSystem が反転描画する
  bool facingRight = false;
};
