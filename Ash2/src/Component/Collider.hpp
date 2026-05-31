#pragma once
#include <Siv3D.hpp>

/// @brief コライダーコンポーネント（カプセル形状）
/// カプセル = 線分 [segmentStart, segmentEnd]（WorldPos 相対、x=w y=h z=d）+
/// radius
struct Collider {
  /// カプセルの始点（WorldPos からのオフセット）
  s3d::Vec3 segmentStart;
  /// カプセルの終点（WorldPos からのオフセット）
  s3d::Vec3 segmentEnd;
  /// カプセルの半径
  double radius;
};
