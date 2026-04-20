#pragma once

#include "Input/InputState.hpp"

/// @brief フレームごとの更新データ
struct FrameData {
  /// 経過時間（秒）
  double dt = 0.0;
  /// 入力状態
  InputState input;
};
