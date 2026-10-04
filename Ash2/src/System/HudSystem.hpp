#pragma once
#include <entt/entt.hpp>

/// @brief 画面固定 HUD 描画システム
///
/// ワールド座標と無関係に画面座標へ直接描画する。
class HudSystem {
 public:
  /// @brief ScreenPos + Drawable エンティティを画面へ描画する
  static void Draw(const entt::registry& registry);
};
