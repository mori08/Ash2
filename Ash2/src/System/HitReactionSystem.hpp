#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "System/HitSystem.hpp"

/// @brief ヒット成立後のヒットストップ付与とリアクション適用を行うシステム
class HitReactionSystem {
 public:
  /// @brief 新たに成立したヒットへヒットストップを付与し、`Enemy`
  /// を持つ被弾側にリアクションを適用する
  ///
  /// `Hp` が枯渇した被弾側は `Attack::reaction` によらず `Defeated`
  /// へ強制遷移し（`Collider`/`Hp` を外す）、それ以外は `reaction`
  /// に応じた `Motion` 遷移と、攻撃側本体との位置関係から決めた向きの
  /// `Velocity` を適用する。
  static void Apply(entt::registry& registry, const Array<HitPair>& hits);
};
