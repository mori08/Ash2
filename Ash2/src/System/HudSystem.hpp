#pragma once
#include <entt/entt.hpp>

#include "Component/Hp.hpp"
#include "Component/Player.hpp"
#include "Component/Stamina.hpp"

/// @brief 画面固定 HUD 描画システム
/// ワールド座標と無関係に画面左上にゲージを描画する
class HudSystem {
 public:
  /// @brief Player + Hp + Stamina を持つエンティティの HP /
  /// スタミナゲージを画面左上に描画する
  /// @param registry ECS レジストリ
  static void Draw(const entt::registry& registry) {
    constexpr double KBarX = 16.0;
    constexpr double KHpBarY = 16.0;
    constexpr double KStaminaBarY = 40.0;
    constexpr double KBarWidth = 200.0;
    constexpr double KBarHeight = 18.0;
    constexpr s3d::ColorF KBgColor{0.2, 0.2, 0.2, 0.7};
    constexpr s3d::ColorF KHpColor{0.2, 0.8, 0.2};
    constexpr s3d::ColorF KStaminaColor{0.9, 0.8, 0.1};

    auto view = registry.view<const Player, const Hp, const Stamina>();
    for (const auto& [entity, hp, stamina] : view.each()) {
      // HP ゲージ
      s3d::RectF{KBarX, KHpBarY, KBarWidth, KBarHeight}.draw(KBgColor);
      if (hp.max > 0) {
        const double hpRatio =
            s3d::Clamp(static_cast<double>(hp.current) / hp.max, 0.0, 1.0);
        s3d::RectF{KBarX, KHpBarY, KBarWidth * hpRatio, KBarHeight}.draw(
            KHpColor);
      }

      // スタミナゲージ
      s3d::RectF{KBarX, KStaminaBarY, KBarWidth, KBarHeight}.draw(KBgColor);
      if (stamina.max > 0) {
        const double staminaRatio = s3d::Clamp(
            static_cast<double>(stamina.current) / stamina.max, 0.0, 1.0);
        s3d::RectF{KBarX, KStaminaBarY, KBarWidth * staminaRatio, KBarHeight}
            .draw(KStaminaColor);
      }

      // Player タグを持つエンティティは 1
      // 体のみ想定のため最初のものだけ描画する
      break;
    }
  }
};
