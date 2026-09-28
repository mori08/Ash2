#include "Phase/StageClearPhase.hpp"

#include "Phase/TestMenuPhase.hpp"
#include "System/BattleSystem.hpp"
#include "UiFonts.hpp"

namespace {
// 画面の見せ方の値（戦闘の調整値ではないため TOML には出さない）
constexpr double kDisplaySec = 2.0;
}  // namespace

PhaseCommand StageClearPhase::update(
    entt::registry& registry, const FrameData& frameData
) {
  BattleSystem::UpdateAftermath(registry, frameData.dt);
  m_elapsed += frameData.dt;
  if (m_elapsed < kDisplaySec) {
    return PhaseCommand::None{};
  }
  // ScenarioPhase を含むスタックを畳み、ステージの実体を確実に片付けるため
  // 遷移先を決め打ちする（ARCHITECTURE.md §3 の例外）
  return PhaseCommand::Reset{
      .nextPhase = std::make_unique<TestMenuPhase>(TestMenuPhase::Param{})
  };
}

void StageClearPhase::draw(const entt::registry& registry) const {
  const auto& font = registry.ctx().get<UiFonts>().large;
  font(U"CLEAR").drawAt(Scene::Center());
}
