#pragma once
#include <entt/entt.hpp>
#include <expected>

#include "Component/WorldPos.hpp"

/// @brief 敵エンティティを生成するファクトリ
class EnemyFactory {
 public:
  /// @brief EnemyFactory::Create の生成パラメータ
  struct Param {
    /// 生成位置
    WorldPos pos{};

    /// @brief TOML から敵の配置パラメータを生成する
    /// @note 敵は接地して出るため h は読まず 0 固定とする
    [[nodiscard]] static std::expected<Param, String> FromToml(
        const TOMLValue& toml
    );
  };

  /// @brief EnemyConfig に基づき、指定位置に敵エンティティを生成する
  static entt::entity Create(entt::registry& registry, const Param& param);
};
