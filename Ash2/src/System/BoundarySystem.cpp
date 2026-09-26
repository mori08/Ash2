#include "System/BoundarySystem.hpp"

#include "Component/Boundary.hpp"
#include "Component/Collider.hpp"
#include "Component/WorldPos.hpp"
#include "Config/ArenaConfig.hpp"

void BoundarySystem::Update(entt::registry& registry) {
  const auto& arena = registry.ctx().get<ArenaConfig>();
  auto view = registry.view<WorldPos, const Boundary, const Collider>();
  for (auto&& [entity, pos, col] : view.each()) {
    const double limitW = arena.halfW - col.radius;
    const double limitD = arena.halfD - col.radius;
    pos.w = Clamp(pos.w, -limitW, limitW);
    pos.d = Clamp(pos.d, -limitD, limitD);
  }
}
