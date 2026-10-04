#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "IPhase.hpp"

/// @brief ステージ定義に従って戦闘を行うフェーズ
///
/// 決着（プレイヤー撃破・敵全滅）を検知したら結果フェーズを Push する。
class StagePhase : public IPhase {
 public:
  /// @brief StagePhase の生成パラメータ
  struct Param {
    /// StageData のキー（ステージ名）
    String stageName;
  };

  explicit StagePhase(const Param& param);

  /// @brief プレイヤーとステージ定義の敵を生成する
  /// @note stageName が StageData に無ければ FatalError を投げる
  void onAfterPush(entt::registry& registry) override;

  /// @brief 戦闘を更新し、決着したら結果フェーズを Push する
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief プレイヤーと残存する敵を破棄する
  void onBeforePop(entt::registry& registry) override;

 private:
  /// StageData のキー
  String m_stageName;
  entt::entity m_playerRoot = entt::null;
  /// ステージ定義から生成した敵（クリア判定の対象）
  Array<entt::entity> m_enemies;
};
