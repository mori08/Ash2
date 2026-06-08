#pragma once

#include "IPhase.hpp"

/// @brief プレイヤー操作デモシーン
class DemoPhase : public IPhase {
 public:
  /// @brief DemoPhase の生成パラメータ（引数なし）
  struct Param {};

  DemoPhase() = default;

  explicit DemoPhase(const Param& /*param*/) : DemoPhase() {}

  /// @brief プレイヤーエンティティ（ルート）を生成する
  void onAfterPush(entt::registry& registry) override;

  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    const FrameData& frameData) override;

  /// @brief プレイヤーエンティティ（ルート＋子孫）を破棄する
  void onBeforePop(entt::registry& registry) override;

 private:
  /// @brief プレイヤーを破棄して最新の設定で再生成する
  void reloadPlayer(entt::registry& registry);

  entt::entity m_playerRoot = entt::null;
};
