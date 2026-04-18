#include "Phase/WaitPhase.hpp"

WaitPhase::WaitPhase(double duration) : m_duration(duration) {}

IPhase::PhaseCommand WaitPhase::update(entt::registry&, double dt) {
  m_elapsed += dt;
  return m_elapsed >= m_duration ? PhaseCommand::Pop() : PhaseCommand::None();
}
