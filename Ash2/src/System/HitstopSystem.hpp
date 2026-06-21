#pragma once
#include <entt/entt.hpp>

/// @brief ヒットストップの経過処理を行うシステム
class HitstopSystem {
 public:
  /// @brief Hitstop を持つエンティティの残り時間を減算し、0
  /// 以下になったら除去する
  static void Update(entt::registry& registry, double dt);
};
