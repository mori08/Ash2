#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Component/WorldPos.hpp"
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

  /// @brief EnemyConfig に基づき、指定位置に敵エンティティを生成する
  entt::entity spawnEnemy(entt::registry& registry, const WorldPos& pos);

  entt::entity m_playerRoot = entt::null;
  entt::entity m_dummyTarget = entt::null;
  /// 敵が撃破され破棄された後、再出現までの残り時間（秒）
  double m_respawnTimer = 0.0;
  /// Key4 のデバッグ操作で追加した敵（撃破されても再生成しない）
  Array<entt::entity> m_extraEnemies;
};
