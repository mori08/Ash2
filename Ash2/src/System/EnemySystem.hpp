#pragma once
#include <entt/entt.hpp>

/// @brief 敵の撃破後処理を行うシステム
class EnemySystem {
 public:
  /// @brief `EnemyMotion::Defeated` の残り時間が尽きたエンティティを破棄する
  static void Update(entt::registry& registry);
};
