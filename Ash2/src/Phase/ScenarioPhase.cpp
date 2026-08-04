#include "Phase/ScenarioPhase.hpp"

#include "Config/ScenarioData.hpp"
#include "Util/Overloaded.hpp"

ScenarioPhase::ScenarioPhase(const Param& param)
    : m_sectionName(param.sectionName) {}

void ScenarioPhase::onAfterPush(entt::registry&) { m_currentStep = 0; }

IPhase::PhaseCommand ScenarioPhase::update(
    entt::registry& registry, const FrameData& /*frameData*/
) {
  const auto& steps =
      registry.ctx().get<ScenarioData>().sections.at(m_sectionName);
  if (m_currentStep >= steps.size()) {
    return PhaseCommand::Pop();
  }

  const auto& step = steps[m_currentStep];
  ++m_currentStep;

  return std::visit(
      Overloaded{
          [&](const StepPush& s) {
            return PhaseCommand::Push(s.maker->make());
          },
          [&](const StepReset& s) {
            return PhaseCommand::Reset(s.maker->make());
          },
      },
      step
  );
}
