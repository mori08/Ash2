#pragma once
#include <ThirdParty/entt/entt.hpp>

/// @brief 親子座標伝播システム
class AttachmentSystem {
 public:
  /// @brief Hierarchy を持つルートエンティティから子孫へ WorldPos を伝播する
  /// @param registry ECS レジストリ
  static void UpdateTransform(entt::registry& registry);
};
