#include "Scene/DemoPhase.hpp"

IPhase::PhaseCommand DemoPhase::update(
    [[maybe_unused]] entt::registry& registry) {
  return PhaseCommand::None();
}
