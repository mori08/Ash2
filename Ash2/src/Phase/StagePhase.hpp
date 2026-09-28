#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "IPhase.hpp"

/// @brief ステージを1つ進行するフェーズ
class StagePhase : public IPhase {
 public:
  /// @brief StagePhase の生成パラメータ
  struct Param {
    /// StageDataRegistry のキー
    String stageName;
  };

  explicit StagePhase(const Param& param);

  /// @brief プレイヤーと敵を生成する
  /// @note m_stageName が StageDataRegistry に無ければ FatalError を投げる
  void onAfterPush(entt::registry& registry) override;

  /// @brief 戦闘を進め、決着したら結果フェーズを Push する
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief プレイヤーと敵を破棄し、独立エンティティを後始末する
  void onBeforePop(entt::registry& registry) override;

 private:
  /// StageDataRegistry のキー
  String m_stageName;
  entt::entity m_player = entt::null;
  /// 生成した敵（決着判定・破棄の対象）
  Array<entt::entity> m_enemies;
};
