#pragma once
#include <Siv3D.hpp>

/// @brief フレームごとのプレイヤー入力状態
/// @note `Key` や `TOMLValue` のようなテストしづらい型は持ち込まない方針だが、
/// `Vec2` のような単純な数学型は許容する
struct InputState {
  /// 移動軸入力（x: 横方向 w 軸・右が正、y: 奥行き方向 d 軸・奥が正）。
  /// 常に長さ 1.0 以下に正規化されていることが保証される
  Vec2 moveAxis = Vec2::Zero();
  bool jumpDown = false;
  bool attackDown = false;
  bool rangedAttackDown = false;
  bool dashDown = false;
};
