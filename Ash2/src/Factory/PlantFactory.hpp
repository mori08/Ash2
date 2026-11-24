#pragma once
#include <entt/entt.hpp>

#include "Config/TitleConfig.hpp"

/// @brief タイトル画面の植物を生成するファクトリ
class PlantFactory {
 public:
  /// @brief 植物1株（親と、株の形の本数ぶんの棒の子）を生成する
  ///
  /// 棒の x には `plant.offset` を反映済み。
  /// @return 親エンティティ。破棄は `Hierarchy::DestroyWithChildren` で行う
  [[nodiscard]] static entt::entity Create(
      entt::registry& registry, const TitleConfig& config,
      const PlantConfig& plant
  );
};
