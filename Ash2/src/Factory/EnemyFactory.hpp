#pragma once
#include <entt/entt.hpp>

#include "Component/WorldPos.hpp"

/// @brief 敵エンティティを生成するファクトリ
class EnemyFactory {
 public:
  /// @brief EnemyFactory::Create の生成パラメータ
  struct Param {
    /// 生成位置
    WorldPos pos{};
  };

  /// @brief EnemyConfig に基づき、指定位置に敵エンティティを生成する
  static entt::entity Create(entt::registry& registry, const Param& param);
};
