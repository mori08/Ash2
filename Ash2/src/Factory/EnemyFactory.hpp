#pragma once
#include <Siv3D.hpp>

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

    /// @brief TOML のテーブルから生成パラメータを生成する
    /// @note `w` / `d` が必須。`h` は持たない（敵は接地で生成する）
    [[nodiscard]] static std::expected<Param, String> FromToml(
        const TOMLValue& toml
    );
  };

  /// @brief EnemyConfig に基づき、指定位置に敵エンティティを生成する
  static entt::entity Create(entt::registry& registry, const Param& param);
};
