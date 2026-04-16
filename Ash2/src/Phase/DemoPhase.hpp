#pragma once

#include "IPhase.hpp"

/// @brief プレイヤー操作デモシーン
class DemoPhase : public IPhase {
 public:
  /// @brief プレイヤーエンティティを生成する
  /// @param registry ECS レジストリ
  void onAfterPush(entt::registry& registry) override;

  /// @brief 毎フレームの更新処理
  /// @param registry ECS レジストリ
  /// @return フェーズスタックへの操作
  [[nodiscard]] PhaseCommand update(entt::registry& registry) override;
};
