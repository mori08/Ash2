#pragma once
#include <entt/entt.hpp>

/// @brief 親子座標伝播システム
class AttachmentSystem {
 public:
  /// @brief Hierarchy を持つルートエンティティから子孫へ WorldPos を伝播する
  static void UpdateTransform(entt::registry& registry);
};
