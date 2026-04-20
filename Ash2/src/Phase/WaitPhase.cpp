#include "Phase/WaitPhase.hpp"

WaitPhase::WaitPhase(double duration) : m_duration(duration) {}

IPhase::PhaseCommand WaitPhase::update(entt::registry&,
                                       const FrameData& frameData) {
  m_elapsed += frameData.dt;
  return m_elapsed >= m_duration ? PhaseCommand::Pop() : PhaseCommand::None();
}
