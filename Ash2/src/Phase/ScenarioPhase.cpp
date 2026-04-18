#include "Phase/ScenarioPhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/Name.hpp"
#include "Component/WorldPos.hpp"
#include "Config/ScenarioData.hpp"
#include "Phase/PhaseRegistry.hpp"
#include "System/NameLookup.hpp"

ScenarioPhase::ScenarioPhase(s3d::String sectionName)
    : m_sectionName(std::move(sectionName)) {}

std::unique_ptr<IPhase> ScenarioPhase::FromToml(const TOMLValue& step) {
  return std::make_unique<ScenarioPhase>(step[U"param"].get<String>());
}

void ScenarioPhase::onAfterPush(entt::registry&) { m_currentStep = 0; }

IPhase::PhaseCommand ScenarioPhase::update(entt::registry& registry,
                                           double /*dt*/) {
  const auto& steps =
      registry.ctx().get<ScenarioData>().sections.at(m_sectionName);
  if (m_currentStep >= steps.size()) {
    return PhaseCommand::Pop();
  }

  const auto& step = steps[m_currentStep];
  ++m_currentStep;
  const auto action = step[U"action"].get<String>();
  const auto& factories = registry.ctx().get<PhaseRegistry>();

  if (action == U"push") {
    return PhaseCommand::Push(factories.at(step[U"phase"].get<String>())(step));
  }

  if (action == U"reset") {
    return PhaseCommand::Reset(
        factories.at(step[U"phase"].get<String>())(step));
  }

  if (action == U"make") {
    const auto name = step[U"name"].get<String>();
    const auto wp = step[U"worldPos"];
    const auto dr = step[U"drawable"];

    auto entity = registry.create();
    m_createdEntities.push_back(entity);

    registry.emplace<Name>(entity, name);
    registry.emplace<WorldPos>(entity, WorldPos{
                                           .w = wp[U"w"].get<double>(),
                                           .h = wp[U"h"].get<double>(),
                                           .d = wp[U"d"].get<double>(),
                                       });
    registry.emplace<Drawable>(
        entity,
        RectDrawable{
            .size = SizeF{dr[U"w"].get<double>(), dr[U"h"].get<double>()},
            .color = ColorF{dr[U"r"].get<double>(), dr[U"g"].get<double>(),
                            dr[U"b"].get<double>()},
        });

    auto& lookup = registry.ctx().get<NameLookup>();
    lookup[name] = entity;

    return PhaseCommand::None();
  }

  return PhaseCommand::None();
}

void ScenarioPhase::onBeforePop(entt::registry& registry) {
  auto& lookup = registry.ctx().get<NameLookup>();
  for (auto entity : m_createdEntities) {
    if (registry.all_of<Name>(entity)) {
      lookup.erase(registry.get<Name>(entity).value);
    }
    registry.destroy(entity);
  }
  m_createdEntities.clear();
}
