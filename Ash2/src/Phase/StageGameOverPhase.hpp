#pragma once
#include <Siv3D.hpp>

#include "IPhase.hpp"

/// @brief プレイヤー撃破時の結果表示フェーズ
/// @note StagePhase だけが積む子フェーズのため、Param を持たず
///       GetPhaseLoaders() にも登録しない
class StageGameOverPhase : public IPhase {
 public:
  /// @brief 決着後の演出だけを進め、表示時間を積算する
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief 画面中央に "GAME OVER" を描く
  void draw(const entt::registry& registry) const override;

 private:
  /// 表示開始からの積算経過時間（秒）
  double m_elapsed = 0.0;
};
