#pragma once
#include <entt/entt.hpp>

#include "Component/WorldPos.hpp"

/// @brief プレイヤーエンティティを生成するファクトリ
class PlayerFactory {
 public:
  /// @brief PlayerFactory::Create の生成パラメータ
  struct Param {
    /// 生成位置
    WorldPos pos{};
  };

  /// @brief プレイヤーエンティティ（ルート）を生成する
  static entt::entity Create(entt::registry& registry, const Param& param);
};
