#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief 攻撃側・被弾側のエンティティの組
struct HitPair {
  entt::entity attacker;
  entt::entity target;
};

/// @brief ヒット判定システム
class HitSystem {
 public:
  /// @brief 攻撃側コライダーと被弾側コライダーの重なりを検出し、Hp
  /// にダメージを適用する
  /// @return このフレームで新たに成立したヒットの一覧
  [[nodiscard]] static Array<HitPair> Update(entt::registry& registry);
};
