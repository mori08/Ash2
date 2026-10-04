#include "Phase/StageClearPhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/ScreenPos.hpp"
#include "FrameData.hpp"
#include "Phase/TestMenuPhase.hpp"
#include "System/BattleSystem.hpp"
#include "UiFonts.hpp"

namespace {
/// 結果を表示してから TestMenuPhase へ戻るまでの秒数
constexpr double kDisplaySec = 3.0;
}  // namespace

void StageClearPhase::onAfterPush(entt::registry& registry) {
  m_elapsed = 0.0;
  m_text = registry.create();
  registry.emplace<ScreenPos>(m_text, ScreenPos{.pos = Scene::CenterF()});
  registry.emplace<Drawable>(
      m_text, TextDrawable{.text = U"CLEAR", .font = FontAsset{UiFonts::kLarge}}
  );
}

PhaseCommand StageClearPhase::update(
    entt::registry& registry, const FrameData& frameData
) {
  BattleSystem::UpdateAftermath(registry, frameData);

  m_elapsed += frameData.dt;
  if (m_elapsed >= kDisplaySec) {
    // TODO(#348): 遷移先を TestMenuPhase に決め打ちしている
    return PhaseCommand::Reset{.nextPhase = std::make_unique<TestMenuPhase>()};
  }
  return PhaseCommand::None{};
}

void StageClearPhase::onBeforePop(entt::registry& registry) {
  if (m_text != entt::null) {
    registry.destroy(m_text);
    m_text = entt::null;
  }
}
