#pragma once
#include <Siv3D.hpp>

#include "Phase/PhaseRegistry.hpp"
#include "Phase/ScenarioStep.hpp"

/// @brief シナリオデータ（ロード時に全セクションを変換済み）
struct ScenarioData {
  /// セクション名 → ステップ一覧のテーブル
  s3d::HashTable<s3d::String, s3d::Array<ScenarioStep>> sections;

  /// @brief TOML からシナリオデータを生成する
  /// @param toml シナリオ TOML のルート値
  /// @param registry フェーズ名 → PhaseEntry の対応表
  /// @return ScenarioData
  [[nodiscard]] static ScenarioData FromToml(const s3d::TOMLValue& toml,
                                             const PhaseRegistry& registry);
};
