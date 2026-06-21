#pragma once
#include <entt/entt.hpp>

/// @brief ひるみリアクションの経過処理を行うシステム
class StaggerSystem {
 public:
  /// @brief Stagger を持つエンティティの残り時間を減算し、RectDrawable::size
  /// を縮み具合に応じて更新する。0 以下になったら originalSize に戻して除去する
  static void Update(entt::registry& registry, double dt);
};
