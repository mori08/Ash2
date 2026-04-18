#include "Phase/WaitPhase.hpp"

WaitPhase::WaitPhase(double duration) : m_duration(duration) {}

std::unique_ptr<IPhase> WaitPhase::FromToml(const TOMLValue& step) {
  return std::make_unique<WaitPhase>(step[U"duration"].get<double>());
}

IPhase::PhaseCommand WaitPhase::update(entt::registry&, double dt) {
  m_elapsed += dt;
  return m_elapsed >= m_duration ? PhaseCommand::Pop() : PhaseCommand::None();
}
