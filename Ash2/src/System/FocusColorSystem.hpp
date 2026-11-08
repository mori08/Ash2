#pragma once
#include <entt/entt.hpp>

/// @brief 選択状態に応じて `DrawColor` を書き換えるシステム
class FocusColorSystem {
 public:
  /// @brief `FocusColor` を持つエンティティの `DrawColor` を更新する
  ///
  /// `Focused` 付きには `focused`、付いていないものには `normal` を書く。
  /// @note `DrawColor` を持たないエンティティには付与する
  static void Update(entt::registry& registry);
};
