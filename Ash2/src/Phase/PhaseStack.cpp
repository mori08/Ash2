#include "Phase/PhaseStack.hpp"

#include "Util/Overloaded.hpp"

PhaseStack::PhaseStack(
    std::unique_ptr<IPhase>&& initialPhase, entt::registry& registry
) {
  push(registry, std::move(initialPhase));
}

void PhaseStack::update(entt::registry& registry, const FrameData& frameData) {
  if (m_stack.empty()) {
    return;
  }

  auto command = m_stack.back()->update(registry, frameData);

  std::visit(
      Overloaded{
          [](const PhaseCommand::None&) {},
          [&](const PhaseCommand::Pop&) { pop(registry); },
          [&](PhaseCommand::Push& cmd) {
            push(registry, std::move(cmd.nextPhase));
          },
          [&](PhaseCommand::Reset& cmd) {
            while (not m_stack.empty()) {
              pop(registry);
            }
            push(registry, std::move(cmd.nextPhase));
          },
      },
      command.value()
  );
}

void PhaseStack::pop(entt::registry& registry) {
  m_stack.back()->onBeforePop(registry);
  m_stack.pop_back();
}

void PhaseStack::push(
    entt::registry& registry, std::unique_ptr<IPhase>&& phase
) {
  assert(phase != nullptr);
  m_stack.push_back(std::move(phase));
  m_stack.back()->onAfterPush(registry);
}
