#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "IPhase.hpp"

/// @brief ステージクリアの結果表示フェーズ
class StageClearPhase : public IPhase {
 public:
  /// @brief StageClearPhase の生成パラメータ（引数なし）
  struct Param {};

  StageClearPhase() = default;

  explicit StageClearPhase(const Param& /*param*/) : StageClearPhase() {}

  /// @brief CLEAR の文字エンティティを生成する
  void onAfterPush(entt::registry& registry) override;

  /// @brief 演出を進め、表示秒数を過ぎたら TestMenuPhase へ Reset する
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief 文字エンティティを破棄する
  void onBeforePop(entt::registry& registry) override;

 private:
  /// CLEAR の文字エンティティ
  entt::entity m_text = entt::null;
  /// 表示開始からの経過時間（秒）
  double m_elapsed = 0.0;
};
