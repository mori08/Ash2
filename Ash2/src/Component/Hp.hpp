#pragma once
#include <Siv3D.hpp>

/// @brief HP コンポーネント
///
/// このコンポーネントを持つエンティティが `Collider`
/// も持つとき、被弾判定の対象になる。
struct Hp {
  int32 max;
  int32 current;
};
