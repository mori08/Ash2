#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "System/HitSystem.hpp"

/// @brief ヒット成立後のヒットストップ付与とリアクション適用を行うシステム
class HitReactionSystem {
 public:
  /// @brief 新たに成立したヒットへヒットストップを付与し、被弾側が持つ
  /// モーション variant（`EnemyMotion::Variant`/`PlayerMotion::Variant`）に
  /// 応じてリアクションを適用する
  ///
  /// `EnemyMotion::Variant` を持つ被弾側は、`Hp` が枯渇していれば `reaction`
  /// によらず `Defeated` へ強制遷移し（`Collider`/`Hp` を外す）、それ以外は
  /// `reaction` に応じた `EnemyMotion::Variant` 遷移と、攻撃側本体との
  /// 位置関係から決めた向きの `Velocity` を適用する。
  /// `PlayerMotion::Variant` を持つ被弾側は、`reaction` が `None` でなければ
  /// `PlayerMotion::MakeDamaged` が `Stagger`/`Knockback` を決めて遷移する。
  static void Apply(entt::registry& registry, const Array<HitEvent>& hits);
};
