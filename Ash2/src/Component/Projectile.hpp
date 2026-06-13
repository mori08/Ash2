#pragma once

/// @brief 飛翔体（弾）エンティティを示すタグコンポーネント
///
/// `WorldPos` + `Velocity` + `Collider` + `Attack` と組み合わせて使用し、
/// `MovementSystem` が移動を、`ProjectileSystem`
/// が消滅を管理する対象を識別する。
struct Projectile {};
