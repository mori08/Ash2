#include "Phase/ScenarioPhase.hpp"

#include <cassert>

#include "Config/ScenarioData.hpp"
#include "Util/Overloaded.hpp"

ScenarioPhase::ScenarioPhase(const Param& param)
    : m_sectionName(param.sectionName) {}

void ScenarioPhase::onAfterPush(entt::registry&) { m_currentStep = 0; }

PhaseCommand ScenarioPhase::update(
    entt::registry& registry, const FrameData& /*frameData*/
) {
  const auto& sections = registry.ctx().get<ScenarioData>().sections;
  const auto it = sections.find(m_sectionName);
  // Release では assert が消えるため、ゲームループ内 throw を避ける
  // フォールバックとして if も残す
  assert(
      it != sections.end() &&
      "セクション名は読み込み時に検証済みでなければならない"
  );
  if (it == sections.end()) {
    return PhaseCommand::Pop{};
  }
  const auto& steps = it->second;
  if (m_currentStep >= steps.size()) {
    return PhaseCommand::Pop{};
  }

  const auto& step = steps[m_currentStep];
  ++m_currentStep;

  return std::visit(
      Overloaded{
          [&](const StepPush& s) -> PhaseCommand {
            return PhaseCommand::Push{.nextPhase = s.maker->make()};
          },
          [&](const StepReset& s) -> PhaseCommand {
            return PhaseCommand::Reset{.nextPhase = s.maker->make()};
          },
      },
      step
  );
}
