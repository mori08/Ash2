#pragma once

/// @brief 重力の影響を受けるエンティティに付与するコンポーネント
struct Gravity {
  /// 重力加速度（ピクセル/秒^2）
  double accel = 0.0;
};
