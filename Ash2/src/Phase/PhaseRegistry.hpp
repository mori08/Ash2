#pragma once

#include <Siv3D.hpp>

#include <functional>
#include <memory>

#include "Phase/PhaseParam.hpp"

class IPhase;

/// @brief フェーズ名とパース・生成関数の対応エントリ
struct PhaseEntry {
  /// @brief TOML 値からフェーズパラメータを生成する関数
  std::function<PhaseParam(const s3d::TOMLValue&)> parseParam;

  /// @brief フェーズパラメータから IPhase インスタンスを生成する関数
  std::function<std::unique_ptr<IPhase>(const PhaseParam&)> createPhase;
};

/// @brief フェーズ名 → PhaseEntry のマップ
using PhaseRegistry = s3d::HashTable<s3d::String, PhaseEntry>;

/// @brief ゲームで使用するフェーズの登録情報を生成する
/// @return フェーズ名と PhaseEntry の対応表
[[nodiscard]] PhaseRegistry MakeDefaultPhaseRegistry();
