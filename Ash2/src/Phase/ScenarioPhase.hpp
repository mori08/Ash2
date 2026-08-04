#pragma once
#include <Siv3D.hpp>

#include "IPhase.hpp"

/// @brief TOML シナリオに従ってエンティティ生成・フェーズ遷移を進めるフェーズ
class ScenarioPhase : public IPhase {
 public:
  /// @brief ScenarioPhase の生成パラメータ
  struct Param {
    /// 処理するシナリオセクション名
    String sectionName;
  };

  explicit ScenarioPhase(const Param& param);

  /// @brief m_currentStep を初期化する
  void onAfterPush(entt::registry& registry) override;

  /// @brief 毎フレーム 1 ステップを処理する
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

 private:
  String m_sectionName;
  size_t m_currentStep = 0;
};
