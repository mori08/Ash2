#pragma once
#include <Siv3D.hpp>

/// @brief コライダーコンポーネント（カプセル形状）
///
/// カプセル = 線分 [segmentStart, segmentEnd]（WorldPos 相対、x=w y=h z=d）+
/// radius
struct Collider {
  /// WorldPos からのオフセット
  Vec3 segmentStart;
  /// WorldPos からのオフセット
  Vec3 segmentEnd;
  double radius;
};
