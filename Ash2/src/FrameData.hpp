#pragma once

#include "Input/InputState.hpp"
#include "RandomSource.hpp"

/// @brief フレームごとの更新データ
struct FrameData {
  /// 経過時間（秒）
  double dt = 0.0;
  InputState input;
  /// 乱数の取り出し口。既定エンジンの取得はここだけで行う
  RandomSource random{GetDefaultRNG()};
};
