#pragma once
#include <Siv3D.hpp>

/// @brief 被弾側に生じるリアクションの強さ
enum class ReactionLevel : uint8 {
  /// Lv0: ひるまない（既定値）
  None,
  /// Lv1: 小さくひるむ
  Stagger,
  /// Lv2: 弾かれる
  Repel,
  /// Lv3: 吹っ飛ぶ
  Blow,
};
