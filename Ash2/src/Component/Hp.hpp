#pragma once

/// @brief HP コンポーネント
///
/// このコンポーネントを持つエンティティが `Collider`
/// も持つとき、被弾判定の対象になる。
struct Hp {
  int max;
  int current;
};
