#pragma once

/// @brief 飛翔体（弾）エンティティを示すタグコンポーネント（データなし）
/// `WorldPos`+`Velocity`+`Collider`+`Attack`
/// と組み合わせて使用し、`ProjectileSystem`
/// が移動・消滅を管理する対象を識別する
struct Projectile {};
