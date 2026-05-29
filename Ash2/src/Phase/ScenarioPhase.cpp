#include "Phase/ScenarioPhase.hpp"

#include "Config/ScenarioData.hpp"
#include "Phase/PhaseParam.hpp"
#include "Phase/PhaseRegistry.hpp"
#include "Phase/ScenarioStep.hpp"
#include "Util/Overloaded.hpp"

ScenarioPhase::ScenarioPhase(const Param& param)
    : m_sectionName(param.sectionName) {}

void ScenarioPhase::onAfterPush(entt::registry&) { m_currentStep = 0; }

IPhase::PhaseCommand ScenarioPhase::update(entt::registry& registry,
                                           const FrameData& /*frameData*/) {
  const auto& steps =
      registry.ctx().get<ScenarioData>().sections.at(m_sectionName);
  if (m_currentStep >= steps.size()) {
    return PhaseCommand::Pop();
  }

  const auto& step = steps[m_currentStep];
  ++m_currentStep;

  const auto& factories = registry.ctx().get<PhaseRegistry>();

  return std::visit(Overloaded{
                        [&](const StepPush& s) {
                          return PhaseCommand::Push(
                              factories.at(s.phaseName).createPhase(s.param));
                        },
                        [&](const StepReset& s) {
                          return PhaseCommand::Reset(
                              factories.at(s.phaseName).createPhase(s.param));
                        },
                    },
                    step);
}
