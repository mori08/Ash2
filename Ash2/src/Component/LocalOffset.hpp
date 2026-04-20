#pragma once
#include "Component/WorldPos.hpp"

/// @brief 親からの相対座標
/// @details Hierarchy を持つエンティティに付ける。WorldPos は常に絶対座標。
struct LocalOffset {
  /// 親からの相対座標
  WorldPos value;
};
