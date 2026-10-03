#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "IPhase.hpp"

/// @brief プレイヤー操作テストフェーズ
class PlayerTestPhase : public IPhase {
 public:
  /// @brief PlayerTestPhase の生成パラメータ（引数なし）
  struct Param {};

  PlayerTestPhase() = default;

  explicit PlayerTestPhase(const Param& /*param*/) : PlayerTestPhase() {}

  /// @brief プレイヤーエンティティ（ルート）と敵エンティティを生成する
  void onAfterPush(entt::registry& registry) override;

  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief プレイヤーエンティティ（ルート＋子孫）と敵エンティティを破棄する
  void onBeforePop(entt::registry& registry) override;

 private:
  /// @brief プレイヤーを破棄して最新の設定で再生成する
  void reloadPlayer(entt::registry& registry);

  entt::entity m_playerRoot = entt::null;
  entt::entity m_dummyTarget = entt::null;
  /// 敵が撃破され破棄された後、再出現までの残り時間（秒）
  double m_respawnTimer = 0.0;
  /// プレイヤー撃破（Dead）検知からの残り時間（秒）。負値は未検知
  double m_deathTimer = -1.0;
  /// 撃破時に表示する GAME OVER の文字エンティティ
  entt::entity m_gameOverText = entt::null;
  /// Key4 のデバッグ操作で追加した敵（撃破されても再生成しない）
  Array<entt::entity> m_extraEnemies;
};
