#pragma once

#include <Siv3D.hpp>

#include "IPhase.hpp"

/// @brief アニメーション再生確認フェーズ
class AnimationViewerPhase : public IPhase {
 public:
  /// @brief AnimationViewerPhase の生成パラメータ
  struct Param {
    /// AnimationDataRegistry のキー（エンティティ種別名）
    String dataKey;
  };

  explicit AnimationViewerPhase(const Param& param);

  /// @brief 一時エンティティを生成する
  void onAfterPush(entt::registry& registry) override;

  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    const FrameData& frameData) override;

  /// @brief 一時エンティティを破棄する
  void onBeforePop(entt::registry& registry) override;

 private:
  static constexpr int KFontSize = 20;

  /// AnimationDataRegistry のキー
  String m_dataKey;
  /// 表示対象エンティティ
  entt::entity m_entity = entt::null;
  /// クリップ名の配列（ソート済み）
  Array<String> m_clips;
  int m_clipIndex = 0;
  Font m_font{KFontSize};
};
