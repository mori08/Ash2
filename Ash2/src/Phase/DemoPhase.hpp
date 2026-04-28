#pragma once

#include "IPhase.hpp"

/// @brief プレイヤー操作デモシーン
class DemoPhase : public IPhase {
 public:
  /// @brief プレイヤーエンティティ（ルート＋パーツ6体）を生成する
  /// @param registry ECS レジストリ
  void onAfterPush(entt::registry& registry) override;

  /// @brief 毎フレームの更新処理
  /// @param registry ECS レジストリ
  /// @param frameData フレームごとの更新データ
  /// @return フェーズスタックへの操作
  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    const FrameData& frameData) override;

  /// @brief プレイヤーエンティティ（ルート＋子孫）を破棄する
  /// @param registry ECS レジストリ
  void onBeforePop(entt::registry& registry) override;

 private:
  /// プレイヤーのルートエンティティ
  entt::entity m_playerRoot = entt::null;
};
