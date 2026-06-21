#pragma once
#include <Siv3D.hpp>

/// @brief ヒットストップ中であることを示すタイマーコンポーネント
///
/// このコンポーネントを持つエンティティは、`HitstopSystem`
/// が経過時間を減算し、0 以下になった時点で除去する。
/// 付与中は `MotionSystem`/`MovementSystem`/`GravitySystem`/`AnimationSystem`
/// の更新がスキップされる。
struct Hitstop {
  /// 残り時間（秒）
  double remaining = 0.0;
};
