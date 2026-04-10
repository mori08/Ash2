#pragma once

/// @brief ワールド空間の速度コンポーネント
struct Velocity {
  /// 横速度（ピクセル/秒）
  double w = 0.0;
  /// 高さ方向速度（ピクセル/秒、上方向が正）
  double h = 0.0;
  /// 奥行き方向速度（ピクセル/秒）
  double d = 0.0;
};
