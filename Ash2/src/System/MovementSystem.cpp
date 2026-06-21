#include "System/MovementSystem.hpp"

#include "Component/Hitstop.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"

void MovementSystem::Update(entt::registry& registry, double dt) {
  auto view = registry.view<WorldPos, Velocity>(entt::exclude<Hitstop>);
  for (auto&& [entity, pos, vel] : view.each()) {
    pos.w += vel.w * dt;
    pos.h += vel.h * dt;
    pos.d += vel.d * dt;
  }
}
