#include "Phase/TitlePhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/ScreenPos.hpp"
#include "Config/TitleConfig.hpp"
#include "FrameData.hpp"
#include "UiFonts.hpp"

void TitlePhase::onAfterPush(entt::registry& registry) {
  m_logo = registry.create();
  const auto& config = registry.ctx().get<TitleConfig>();
  registry.emplace<ScreenPos>(
      m_logo, ScreenPos{.pos = Vec2{Scene::CenterF().x, config.logoY}}
  );
  registry.emplace<Drawable>(
      m_logo,
      TextDrawable{.text = U"沼に焚べ", .font = FontAsset{UiFonts::kLarge}}
  );
}

PhaseCommand TitlePhase::update(
    entt::registry& /*registry*/, const FrameData& /*frameData*/
) {
  return PhaseCommand::None{};
}

void TitlePhase::onBeforePop(entt::registry& registry) {
  if (m_logo != entt::null) {
    registry.destroy(m_logo);
    m_logo = entt::null;
  }
}
