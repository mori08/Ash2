#include "System/MotionSystem.hpp"

#include "Component/Hitstop.hpp"
#include "Phase/FrameData.hpp"
#include "System/PlayerMotionSystem.hpp"

void MotionSystem::Update(entt::registry& registry,
                          const FrameData& frameData) {
  auto view = registry.view<Motion>(entt::exclude<Hitstop>);
  for (const auto entity : view) {
    auto& motion = view.get<Motion>(entity);
    auto next = std::visit(
        [&](auto& state) { return Tick(state, registry, entity, frameData); },
        motion);
    if (next.has_value()) {
      registry.replace<Motion>(entity, *next);
    }
  }
}
