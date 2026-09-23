#include "System/BoundarySystem.hpp"

#include "Component/Boundary.hpp"
#include "Component/Collider.hpp"
#include "Component/WorldPos.hpp"
#include "Config/StageConfig.hpp"

void BoundarySystem::Update(entt::registry& registry) {
  const auto& stage = registry.ctx().get<StageConfig>();
  auto view = registry.view<WorldPos, const Boundary, const Collider>();
  for (auto&& [entity, pos, col] : view.each()) {
    const double limitW = stage.halfW - col.radius;
    const double limitD = stage.halfD - col.radius;
    pos.w = Clamp(pos.w, -limitW, limitW);
    pos.d = Clamp(pos.d, -limitD, limitD);
  }
}
