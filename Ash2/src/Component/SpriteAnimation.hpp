#pragma once
#include <Siv3D.hpp>

/// @brief スプライトアニメーションコンポーネント（per-entity、軽量）
struct SpriteAnimation {
  /// AnimationDataRegistry のキー（アニメーション設定ファイル名由来）
  s3d::String dataKey;
  /// 現在再生中のクリップ名
  s3d::String currentClip;
  /// 現クリップの再生経過時間（秒）
  double elapsed = 0.0;
  /// 右向きかどうか（true のとき左右反転して描画）
  bool facingRight = false;
};
