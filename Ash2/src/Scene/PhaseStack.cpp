#include "Scene/PhaseStack.hpp"

PhaseStack::PhaseStack(std::unique_ptr<IPhase>&& initialPhase,
                       entt::registry& registry) {
  push(registry, std::move(initialPhase));
}

void PhaseStack::update(entt::registry& registry) {
  if (m_stack.empty()) {
    return;
  }

  auto command = m_stack.back()->update(registry);

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
      while (not m_stack.empty()) {
        pop(registry);
      }
      push(registry, std::move(command.nextPhase));
      break;
  }
}

void PhaseStack::pop(entt::registry& registry) {
  m_stack.back()->onBeforePop(registry);
  m_stack.pop_back();
}

void PhaseStack::push(entt::registry& registry,
                      std::unique_ptr<IPhase>&& phase) {
  m_stack.push_back(std::move(phase));
  m_stack.back()->onAfterPush(registry);
}
