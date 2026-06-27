#pragma once
#include <entt/entt.hpp>

/// @brief スタミナ回復の処理を行うシステム
class StaminaSystem {
 public:
  /// @brief Neutral 状態のみ、recoveryDelay 秒待機後にスタミナを回復する
  static void Update(entt::registry& registry, double dt);
};
