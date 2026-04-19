#include <Siv3D.hpp>

#include "Phase/DemoPhase.hpp"
#include "Phase/PhaseRegistry.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "Phase/WaitPhase.hpp"

PhaseRegistry MakeDefaultPhaseRegistry() {
  return {
      {U"demo", [](const TOMLValue&) { return std::make_unique<DemoPhase>(); }},
      {U"scenario",
       [](const TOMLValue& step) {
         return std::make_unique<ScenarioPhase>(step[U"param"].get<String>());
       }},
      {U"wait",
       [](const TOMLValue& step) {
         return std::make_unique<WaitPhase>(step[U"duration"].get<double>());
       }},
  };
}
