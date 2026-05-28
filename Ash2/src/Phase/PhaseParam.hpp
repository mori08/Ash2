#pragma once

#include <variant>

#include "Phase/DemoPhase.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "Phase/WaitPhase.hpp"

/// @brief 各フェーズの生成パラメータをまとめた variant 型
using PhaseParam =
    std::variant<WaitPhase::Param, ScenarioPhase::Param, DemoPhase::Param>;
