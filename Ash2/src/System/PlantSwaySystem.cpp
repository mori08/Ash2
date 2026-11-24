#include <Siv3D.hpp>

#include "System/PlantSwaySystem.hpp"

#include "Component/DrawPos.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Plant.hpp"

void PlantSwaySystem::Place(
    entt::registry& registry, entt::entity plantEntity
) {
  const auto& plant = registry.get<const Plant>(plantEntity);
  const double rootX = registry.get<const DrawPos>(plantEntity).pos.x;
  const double angle =
      Math::TwoPi * (plant.elapsed / plant.period + plant.offset);
  const double spread = 1.0 + plant.spreadAmplitude * Sin(angle);

  for (const auto child : Hierarchy::Children(registry, plantEntity)) {
    const auto& stem = registry.get<const PlantStem>(child);
    registry.get<DrawPos>(child).pos.x = rootX + stem.dx * spread;
  }
}

void PlantSwaySystem::Update(entt::registry& registry, double dt) {
  for (const auto entity : registry.view<Plant, const DrawPos>()) {
    registry.get<Plant>(entity).elapsed += dt;
    Place(registry, entity);
  }
}
