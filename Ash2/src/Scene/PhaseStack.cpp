#include "Scene/PhaseStack.hpp"

PhaseStack::PhaseStack(std::unique_ptr<IPhase>&& initialPhase,
                       entt::registry& registry) {
  push(registry, std::move(initialPhase));
}

void PhaseStack::update(entt::registry& registry) {
  if (stack_.empty()) {
    return;
  }

  auto command = stack_.back()->update(registry);

  switch (command.type) {
    case IPhase::PhaseCommand::Type::None:
      break;

    case IPhase::PhaseCommand::Type::Pop:
      pop(registry);
      break;

    case IPhase::PhaseCommand::Type::Push:
      push(registry, std::move(command.nextPhase));
      break;

    case IPhase::PhaseCommand::Type::Reset:
      while (not stack_.empty()) {
        pop(registry);
      }
      push(registry, std::move(command.nextPhase));
      break;
  }
}

void PhaseStack::pop(entt::registry& registry) {
  stack_.back()->onBeforePop(registry);
  stack_.pop_back();
}

void PhaseStack::push(entt::registry& registry,
                      std::unique_ptr<IPhase>&& phase) {
  stack_.push_back(std::move(phase));
  stack_.back()->onAfterPush(registry);
}
