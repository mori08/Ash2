#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "IPhase.hpp"
#include "System/HitSystem.hpp"

/// @brief プレイヤー操作テストフェーズ
class PlayerTestPhase : public IPhase {
 public:
  /// @brief PlayerTestPhase の生成パラメータ（引数なし）
  struct Param {};

  PlayerTestPhase() = default;

  explicit PlayerTestPhase(const Param& /*param*/) : PlayerTestPhase() {}

  /// @brief プレイヤーエンティティ（ルート）を生成する
  void onAfterPush(entt::registry& registry) override;

  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    const FrameData& frameData) override;

  /// @brief プレイヤーエンティティ（ルート＋子孫）を破棄する
  void onBeforePop(entt::registry& registry) override;

 private:
  /// @brief プレイヤーを破棄して最新の設定で再生成する
  void reloadPlayer(entt::registry& registry);

  /// @brief ヒット成立した攻撃側・被弾側へヒットストップ・ひるみを付与する
  ///
  /// 本格的なリアクション設計は #132/#134 のスコープであり、ここでは
  /// 近接1段目の操作感確認に必要な最小限の暫定実装を行う。
  void applyHitReactions(entt::registry& registry, const Array<HitPair>& hits);

  entt::entity m_playerRoot = entt::null;
  entt::entity m_dummyTarget = entt::null;
};
