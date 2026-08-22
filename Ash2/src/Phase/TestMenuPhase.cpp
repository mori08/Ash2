#include "Phase/TestMenuPhase.hpp"

#include "Phase/AnimationViewerPhase.hpp"
#include "Phase/PlayerTestPhase.hpp"
#include "UiFonts.hpp"

namespace {
constexpr double kBgBrightness = 0.1;
constexpr int32 kTitleX = 40;
constexpr int32 kTitleY = 40;
constexpr int32 kItemX = 60;
constexpr int32 kItemBaseY = 100;
constexpr int32 kItemSpacing = 40;
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

PhaseCommand TestMenuPhase::update(
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
    return PhaseCommand::Push{.nextPhase = std::move(phase)};
  }

  const auto& font = registry.ctx().get<UiFonts>().large;

  Scene::SetBackground(ColorF{kBgBrightness});
  font(U"Test Menu").draw(kTitleX, kTitleY);

  for (size_t i = 0; i < m_items.size(); ++i) {
    const ColorF color =
        (i == m_selectedIndex) ? Palette::Yellow : Palette::White;
    font(m_items[i].label)
        .draw(
            kItemX, kItemBaseY + static_cast<double>(i) * kItemSpacing, color
        );
  }

  return PhaseCommand::None{};
}
