#include "System/ProjectionSystem.hpp"

#include "Component/DrawPos.hpp"
#include "Component/WorldPos.hpp"
#include "Screen.hpp"

void ProjectionSystem::Update(entt::registry& registry) {
  for (const auto [entity, worldPos, drawPos] :
       registry.view<const WorldPos, DrawPos>().each()) {
    drawPos.pos = WorldToScreen(worldPos);
    drawPos.depth = worldPos.d;
  }
}
