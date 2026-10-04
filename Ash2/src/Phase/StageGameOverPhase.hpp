#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "IPhase.hpp"

/// @brief プレイヤー撃破の結果表示フェーズ
class StageGameOverPhase : public IPhase {
 public:
  /// @brief StageGameOverPhase の生成パラメータ（引数なし）
  struct Param {};

  StageGameOverPhase() = default;

  explicit StageGameOverPhase(const Param& /*param*/) : StageGameOverPhase() {}

  /// @brief GAME OVER の文字エンティティを生成する
  void onAfterPush(entt::registry& registry) override;

  /// @brief 演出を進め、表示秒数を過ぎたら TestMenuPhase へ Reset する
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief 文字エンティティを破棄する
  void onBeforePop(entt::registry& registry) override;

 private:
  /// GAME OVER の文字エンティティ
  entt::entity m_text = entt::null;
  /// 表示開始からの経過時間（秒）
  double m_elapsed = 0.0;
};
