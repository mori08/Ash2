#include "System/HitstopSystem.hpp"

#include "Component/Hitstop.hpp"

void HitstopSystem::Update(entt::registry& registry, double dt) {
  auto view = registry.view<Hitstop>();
  for (auto&& [entity, hitstop] : view.each()) {
    hitstop.remaining -= dt;
  }

  for (const auto entity : view) {
    if (view.get<Hitstop>(entity).remaining <= 0.0) {
      registry.remove<Hitstop>(entity);
    }
  }
}
