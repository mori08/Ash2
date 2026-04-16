#pragma once
#include <Siv3D.hpp>

/// @brief シナリオデータ（ロード時に全セクションを変換済み）
struct ScenarioData {
  /// セクション名 → ステップ一覧のテーブル
  s3d::HashTable<s3d::String, s3d::Array<s3d::TOMLValue>> sections;

  /// @brief TOML からシナリオデータを生成する
  /// @param toml シナリオ TOML のルート値
  /// @return ScenarioData
  [[nodiscard]] static ScenarioData FromToml(const s3d::TOMLValue& toml);
};
