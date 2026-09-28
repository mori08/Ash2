#pragma once
#include <Siv3D.hpp>

#include <expected>

#include "Factory/EnemyFactory.hpp"

/// @brief 1ステージ分の配置データ
/// @note StagePhase と、読み込みを担う GameSetup だけが使う。Config/ に
///       置かない理由は ARCHITECTURE.md の「ディレクトリ構成」参照。
struct StageData {
  /// 敵の配置（1体以上）
  Array<EnemyFactory::Param> enemies;

  /// @brief TOML の1ステージ分のテーブルからステージデータを生成する
  /// @note enemies が無いか空の場合は失敗を返す（開始直後にクリア扱いに
  ///       なるのを防ぐため）
  [[nodiscard]] static std::expected<StageData, String> FromToml(
      const TOMLValue& table
  );
};

/// @brief ステージ名 → StageData のレジストリ（registry.ctx() に格納）
using StageDataRegistry = HashTable<String, StageData>;
