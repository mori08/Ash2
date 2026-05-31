#pragma once

/// @brief HP コンポーネント
/// このコンポーネントを持つエンティティが Collider
/// も持つとき、被弾判定の対象になる
struct Hp {
  /// 最大 HP
  int max;
  /// 現在 HP
  int current;
};
