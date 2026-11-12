#pragma once
#include <entt/entt.hpp>

/// @brief ワールド座標を描画座標へ投影するシステム
///
/// ワールド座標から画面座標への変換をここ1箇所に集める。
class ProjectionSystem {
 public:
  /// @brief WorldPos + DrawPos を持つエンティティの DrawPos を書き換える
  ///
  /// `pos` に画面座標、`depth` に `WorldPos::d` を書く。`layer` は触らない。
  /// DrawPos だけを持つエンティティ（HUD・メニュー）は対象外。
  /// @note アタッチ伝播（AttachmentSystem）の後、描画の前に呼ぶ
  static void Update(entt::registry& registry);
};
