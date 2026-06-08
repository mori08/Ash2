#pragma once
#include "Component/WorldPos.hpp"

/// @brief 親からの相対座標
///
/// Hierarchy を持つエンティティに付ける。WorldPos は常に絶対座標。
struct LocalOffset {
  WorldPos value;
};
