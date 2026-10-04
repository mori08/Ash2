#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief GaugeSource が指す Hp の割合を、自身の RectDrawable 幅へ反映する
struct HpGauge {
  /// 割合が 1 のときの幅（px）
  double fullWidth;
};

/// @brief GaugeSource が指す Stamina の割合を、自身の RectDrawable 幅へ反映する
struct StaminaGauge {
  /// 割合が 1 のときの幅（px）
  double fullWidth;
};

/// @brief ゲージが値を読む先のエンティティ
///
/// 参照先の生成・破棄には関与しない。参照先より先にゲージを破棄すること。
struct GaugeSource {
  entt::entity entity = entt::null;
};
