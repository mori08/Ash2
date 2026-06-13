#pragma once
#include <entt/entt.hpp>

/// @brief 重力加速と地面クランプを行うシステム
class GravitySystem {
 public:
  /// @brief WorldPos + Velocity + Gravity を持つエンティティに重力を適用する
  ///
  /// `vel.h -= gravity.accel * dt`
  /// により次フレーム用の速度を更新したのち、`pos.h < 0`
  /// の場合は今フレームの `pos.h` と `vel.h` を 0 にクランプする
  /// （地面への沈み込み防止）。重力加速と地面クランプは時間軸の異なる
  /// 処理（次フレーム用 / 今フレーム確定）であり、分割しないこと。
  static void Update(entt::registry& registry, double dt);
};
