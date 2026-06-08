#pragma once

#include <Siv3D.hpp>

#include <variant>

#include "Phase/PhaseParam.hpp"

/// @brief シナリオのステップ：フェーズをスタックに積む
struct StepPush {
  /// PhaseRegistry のキー
  s3d::String phaseName;
  PhaseParam param;
};

/// @brief シナリオのステップ：スタックをリセットして新フェーズを積む
struct StepReset {
  /// PhaseRegistry のキー
  s3d::String phaseName;
  PhaseParam param;
};

/// @brief シナリオの 1 ステップを表す variant 型
using ScenarioStep = std::variant<StepPush, StepReset>;
