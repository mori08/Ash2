#pragma once
#include <entt/entt.hpp>

#include "Component/Hp.hpp"
#include "Component/Player.hpp"
#include "Component/Stamina.hpp"

/// @brief 画面固定 HUD 描画システム
///
/// ワールド座標と無関係に画面左上にゲージを描画する。
class HudSystem {
 public:
  /// @brief Player + Hp + Stamina を持つ最初の1体の HP /
  /// スタミナゲージを画面左上に描画する
  static void Draw(const entt::registry& registry) {
    constexpr double kBarX = 16.0;
    constexpr double kHpBarY = 16.0;
    constexpr double kStaminaBarY = 40.0;
    constexpr double kBarWidth = 200.0;
    constexpr double kBarHeight = 18.0;
    constexpr ColorF kBgColor{0.2, 0.2, 0.2, 0.7};
    constexpr ColorF kHpColor{0.2, 0.8, 0.2};
    constexpr ColorF kStaminaColor{0.9, 0.8, 0.1};

    const auto view = registry.view<const Player, const Hp, const Stamina>();
    const auto entity = view.front();
    if (entity == entt::null) return;

    const auto& hp = view.get<const Hp>(entity);
    const auto& stamina = view.get<const Stamina>(entity);

    RectF{kBarX, kHpBarY, kBarWidth, kBarHeight}.draw(kBgColor);
    if (hp.max > 0) {
      const double hpRatio =
          Clamp(static_cast<double>(hp.current) / hp.max, 0.0, 1.0);
      RectF{kBarX, kHpBarY, kBarWidth * hpRatio, kBarHeight}.draw(kHpColor);
    }

    RectF{kBarX, kStaminaBarY, kBarWidth, kBarHeight}.draw(kBgColor);
    if (stamina.max > 0) {
      const double staminaRatio =
          Clamp(static_cast<double>(stamina.current) / stamina.max, 0.0, 1.0);
      RectF{kBarX, kStaminaBarY, kBarWidth * staminaRatio, kBarHeight}.draw(
          kStaminaColor
      );
    }
  }
};
