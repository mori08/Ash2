#pragma once

#include <Siv3D.hpp>

#include <variant>

#include "Phase/PhaseParam.hpp"

/// @brief シナリオのステップ：フェーズをスタックに積む
struct StepPush {
  /// フェーズ名（PhaseRegistry のキー）
  s3d::String phaseName;
  /// フェーズの生成パラメータ
  PhaseParam param;
};

/// @brief シナリオのステップ：スタックをリセットして新フェーズを積む
struct StepReset {
  /// フェーズ名（PhaseRegistry のキー）
  s3d::String phaseName;
  /// フェーズの生成パラメータ
  PhaseParam param;
};

/// @brief シナリオの 1 ステップを表す variant 型
using ScenarioStep = std::variant<StepPush, StepReset>;
