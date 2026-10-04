#pragma once
#include <Siv3D.hpp>

#include <expected>

#include "Factory/EnemyFactory.hpp"

/// @brief 1 ステージ分の定義
struct StageDefinition {
  /// 敵の配置（定義の並びどおりに生成する。空は許さない）
  Array<EnemyFactory::Param> enemies;
};

/// @brief ステージ定義（ステージ名 → 定義）
/// @note `EnemyFactory::Param` を持つため Config ではなく Factory に置く
struct StageData {
  /// ステージ名 → ステージ定義のテーブル
  HashTable<String, StageDefinition> stages;

  /// @brief TOML からステージ定義を生成する
  /// @note 1 セクションが 1 ステージ。`enemies`
  /// が欠落・空のステージは失敗とする
  [[nodiscard]] static std::expected<StageData, String> FromToml(
      const TOMLValue& toml
  );
};
