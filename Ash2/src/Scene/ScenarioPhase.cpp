#include "Scene/ScenarioPhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/Name.hpp"
#include "Config/ScenarioData.hpp"
#include "System/NameLookup.hpp"
#include "WorldPos.hpp"

ScenarioPhase::ScenarioPhase(const s3d::String& sectionName)
    : sectionName_(sectionName) {}

void ScenarioPhase::onAfterPush(entt::registry&) { currentStep_ = 0; }

IPhase::PhaseCommand ScenarioPhase::update(entt::registry& registry) {
  const auto& steps =
      registry.ctx().get<ScenarioData>().sections.at(sectionName_);
  if (currentStep_ >= steps.size()) {
    return PhaseCommand::Pop();
  }

  const auto& step = steps[currentStep_];
  ++currentStep_;
  const auto action = step[U"action"].get<String>();

  if (action == U"make") {
    const auto name = step[U"name"].get<String>();
    const auto wp = step[U"worldPos"];
    const auto dr = step[U"drawable"];

    auto entity = registry.create();
    createdEntities_.push_back(entity);

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

  if (action == U"reset") {
    const auto param = step[U"param"].get<String>();
    return PhaseCommand::Reset(std::make_unique<ScenarioPhase>(param));
  }

  return PhaseCommand::None();
}

void ScenarioPhase::onBeforePop(entt::registry& registry) {
  auto& lookup = registry.ctx().get<NameLookup>();
  for (auto entity : createdEntities_) {
    if (registry.all_of<Name>(entity)) {
      lookup.erase(registry.get<Name>(entity).value);
    }
    registry.destroy(entity);
  }
  createdEntities_.clear();
}
