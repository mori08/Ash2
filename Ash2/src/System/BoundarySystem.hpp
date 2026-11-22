#pragma once
#include <entt/entt.hpp>

/// @brief アリーナ境界の内側へ位置をクランプするシステム
class BoundarySystem {
 public:
  /// @brief WorldPos + Boundary + Collider を持つエンティティの w/d を
  /// ArenaConfig の境界内へ Clamp する
  ///
  /// @note 時間で進む処理ではなく、同じ入力に何度かけても結果が変わらない
  /// クランプのため、Hitstop を持つエンティティも除外しない。
  /// `MovementSystem` の後、`AttachmentSystem` の前に呼ぶ。
  /// 移動後の位置を子へ伝播する前にクランプするため
  static void Update(entt::registry& registry);
};
