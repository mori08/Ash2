#pragma once

/// @brief スタミナコンポーネント
struct Stamina {
  int max;
  int current;
  /// StaminaSystem が管理する回復端数の累積値
  double accum = 0.0;
  /// Neutral 状態が継続した経過時間（回復ディレイ計測用）
  double recoveryTimer = 0.0;
};
