#pragma once
#include <entt/entt.hpp>

/// @brief ゲージの割合を RectDrawable の幅へ反映するシステム
class GaugeSystem {
 public:
  /// @brief HpGauge / StaminaGauge の幅を、GaugeSource が指す値の割合に更新する
  /// @note GaugeSource の参照先は有効で、対応する Hp / Stamina を持つこと
  static void Update(entt::registry& registry);
};
