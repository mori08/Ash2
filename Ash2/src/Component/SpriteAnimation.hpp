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

/// @brief クリップが変化していれば差し替え、再生位置をリセットする
///
/// 同名クリップなら再生位置を保つため、状態の維持（idle / move など）で
/// 毎フレーム呼んでよい。行動の開始には RestartClip を使う。
inline void SetClip(SpriteAnimation& anim, const String& clip) {
  if (clip != anim.currentClip) {
    anim.currentClip = clip;
    anim.elapsed = 0.0;
  }
}

/// @brief クリップ名が同じでも再生位置を先頭へ強制的に戻す
///
/// 行動の開始（Make* など）で使う。毎フレーム呼ぶと再生が進まなくなるため、
/// 状態の維持には SetClip を使う。
inline void RestartClip(SpriteAnimation& anim, const String& clip) {
  anim.currentClip = clip;
  anim.elapsed = 0.0;
}
