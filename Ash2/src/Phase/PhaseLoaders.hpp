#pragma once
#include <Siv3D.hpp>

#include "Config/ScenarioData.hpp"

/// @brief 実装済みフェーズ名 → PhaseLoader のマップを返す
[[nodiscard]] const PhaseLoaderTable& GetPhaseLoaders();
