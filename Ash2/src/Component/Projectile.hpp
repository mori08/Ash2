#pragma once
#include <Siv3D.hpp>

#include "Component/WorldPos.hpp"

/// @brief 飛翔体（弾）エンティティを示すコンポーネント
///
/// `WorldPos` + `Velocity` + `Collider` + `Attack` と組み合わせて使用し、
/// `MovementSystem` が移動を、`ProjectileSystem`
/// が消滅を管理する対象を識別する。
struct Projectile {
  /// 発射位置（最大射程の判定に使う）
  WorldPos origin;
  /// 最大射程（ピクセル）。既定は無制限
  double maxRange = Math::Inf;
};
