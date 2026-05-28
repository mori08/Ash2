#include "Phase/WaitPhase.hpp"

WaitPhase::WaitPhase(const Param& param) : m_duration(param.duration) {}

IPhase::PhaseCommand WaitPhase::update(entt::registry&,
                                       const FrameData& frameData) {
  m_elapsed += frameData.dt;
  return m_elapsed >= m_duration ? PhaseCommand::Pop() : PhaseCommand::None();
}
