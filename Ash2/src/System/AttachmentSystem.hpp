#pragma once
#include <entt/entt.hpp>

/// @brief 親子座標伝播システム
class AttachmentSystem {
 public:
  /// @brief Hierarchy を持つルートエンティティから子孫へ WorldPos を伝播する
  /// @note `MotionSystem` の後、`HitSystem` の前に呼ぶ。
  ///       Motion が更新した子の `LocalOffset` を判定前に反映するため
  static void UpdateTransform(entt::registry& registry);
};
