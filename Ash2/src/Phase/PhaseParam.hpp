#pragma once

#include <variant>

#include "Phase/AnimationViewerPhase.hpp"
#include "Phase/PlayerTestPhase.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "Phase/TestMenuPhase.hpp"
#include "Phase/WaitPhase.hpp"

/// @brief 各フェーズの生成パラメータをまとめた variant 型
using PhaseParam =
    std::variant<WaitPhase::Param, ScenarioPhase::Param, PlayerTestPhase::Param,
                 TestMenuPhase::Param, AnimationViewerPhase::Param>;
