#pragma once
#include <entt/entt.hpp>

/// @brief 植物の棒の横間隔を周期的に揺らすシステム
class PlantSwaySystem {
 public:
  /// @brief 経過時間を `dt` 進め、子の棒の x を書き換える
  static void Update(entt::registry& registry, double dt);

  /// @brief 時間を進めずに、現在の経過時間で子の棒の x を書き込む
  /// @note 生成直後に `offset` を反映するために使う
  static void Place(entt::registry& registry, entt::entity plantEntity);
};
