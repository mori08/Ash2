#include "Phase/TestMenuPhase.hpp"

#include "Phase/AnimationViewerPhase.hpp"
#include "Phase/PlayerTestPhase.hpp"

namespace {
constexpr double KBgBrightness = 0.1;
constexpr int32 KTitleX = 40;
constexpr int32 KTitleY = 40;
constexpr int32 KItemX = 60;
constexpr int32 KItemBaseY = 100;
constexpr int32 KItemSpacing = 40;
}  // namespace

void TestMenuPhase::onAfterPush(entt::registry& /*registry*/) {
  m_items = {
      MenuItem{
          .label = U"PlayerTest",
          .create = [](entt::registry&) -> std::unique_ptr<IPhase> {
            return std::make_unique<PlayerTestPhase>();
          },
      },
      MenuItem{
          .label = U"AnimationViewer (player)",
          .create = [](entt::registry&) -> std::unique_ptr<IPhase> {
            return std::make_unique<AnimationViewerPhase>(
                AnimationViewerPhase::Param{.dataKey = U"player"}
            );
          },
      },
  };
  m_selectedIndex = 0;
}

IPhase::PhaseCommand TestMenuPhase::update(
    entt::registry& registry, const FrameData& /*frameData*/
) {
  if (KeyUp.down()) {
    m_selectedIndex = (m_selectedIndex - 1 + m_items.size()) % m_items.size();
  }
  if (KeyDown.down()) {
    m_selectedIndex = (m_selectedIndex + 1) % m_items.size();
  }

  if (KeyEnter.down() && !m_items.empty()) {
    auto phase = m_items[m_selectedIndex].create(registry);
    return PhaseCommand::Push(std::move(phase));
  }

  Scene::SetBackground(ColorF{KBgBrightness});
  m_font(U"Test Menu").draw(KTitleX, KTitleY);

  for (size_t i = 0; i < m_items.size(); ++i) {
    const ColorF color =
        (i == m_selectedIndex) ? Palette::Yellow : Palette::White;
    m_font(m_items[i].label)
        .draw(
            KItemX, KItemBaseY + static_cast<double>(i) * KItemSpacing, color
        );
  }

  return PhaseCommand::None();
}
