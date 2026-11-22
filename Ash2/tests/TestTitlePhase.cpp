#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"
#include "Component/FocusColor.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Menu.hpp"
#include "Config/TitleConfig.hpp"
#include "FrameData.hpp"
#include "Phase/TitlePhase.hpp"

namespace {

const ColorF kBackgroundColor{0.9};
const ColorF kButtonNormalColor{0.5};
const ColorF kButtonFocusedColor{0.1, 0.2, 0.3};

/// @brief TitleConfig を ctx に積んだ registry で TitlePhase を push 済みにする
void PushTitle(entt::registry& registry, TitlePhase& phase) {
  registry.ctx().emplace<TitleConfig>(TitleConfig{
      .logoY = 160.0,
      .menuY = 440.0,
      .menuSpacing = 60.0,
      .buttonWidth = 200.0,
      .buttonHeight = 48.0,
      .backgroundColor = kBackgroundColor,
      .surfaceLineColor = ColorF{0.1},
      .logoColor = ColorF{0.2},
      .buttonNormalColor = kButtonNormalColor,
      .buttonFocusedColor = kButtonFocusedColor,
      .surfaceY = 360.0,
      .surfaceLineWidth = 1.5,
  });
  phase.onAfterPush(registry);
}

/// @brief ラベルの文字列からメニューのボタンを探す
entt::entity FindButton(const entt::registry& registry, StringView label) {
  for (const auto entity : registry.view<const Button, const Drawable>()) {
    const auto& drawable = registry.get<const Drawable>(entity);
    if (std::get<TextDrawable>(drawable).text == label) return entity;
  }
  return entt::null;
}

FrameData WithInput(const InputState& input) {
  return FrameData{.dt = 1.0 / 60.0, .input = input};
}

}  // namespace

TEST_CASE("TitlePhase::onAfterPush - creates a menu with two buttons") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);

  const auto start = FindButton(registry, U"はじめる");
  const auto quit = FindButton(registry, U"おわる");
  REQUIRE(start != entt::entity{entt::null});
  REQUIRE(quit != entt::entity{entt::null});
  REQUIRE(registry.get<Button>(start).down == quit);
  REQUIRE(registry.get<Button>(quit).up == start);
  REQUIRE_FALSE(registry.view<Menu>().empty());
}

TEST_CASE("TitlePhase::onAfterPush - creates background behind surface line") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);

  // kBackground の矩形を、色（設定値）で背景と水面線に見分ける
  Optional<DrawPos> background;
  Optional<DrawPos> surfaceLine;
  for (const auto entity :
       registry.view<const DrawPos, const Drawable, const DrawColor>()) {
    const auto& drawPos = registry.get<const DrawPos>(entity);
    if (drawPos.layer != DrawLayer::kBackground) continue;
    REQUIRE(
        std::holds_alternative<RectDrawable>(
            registry.get<const Drawable>(entity)
        )
    );
    if (registry.get<const DrawColor>(entity).color == kBackgroundColor) {
      background = drawPos;
    } else {
      surfaceLine = drawPos;
    }
  }

  REQUIRE(background.has_value());
  REQUIRE(surfaceLine.has_value());
  REQUIRE(background.value().depth > surfaceLine.value().depth);
}

TEST_CASE("TitlePhase::update - navigate focuses the start button") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);

  const auto command = phase.update(
      registry, WithInput(InputState{.navigate = NavDirection::Down})
  );

  const auto start = FindButton(registry, U"はじめる");
  const auto quit = FindButton(registry, U"おわる");
  REQUIRE(std::holds_alternative<PhaseCommand::None>(command.value()));
  REQUIRE(registry.all_of<Focused>(start));
  REQUIRE(registry.get<DrawColor>(start).color == kButtonFocusedColor);
  REQUIRE(registry.get<DrawColor>(quit).color == kButtonNormalColor);
}

TEST_CASE("TitlePhase::update - clicking start returns Push") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);

  const auto pos = registry.get<DrawPos>(FindButton(registry, U"はじめる")).pos;
  const auto command = phase.update(
      registry, WithInput(InputState{.pointerPos = pos, .clickDown = true})
  );

  REQUIRE(std::holds_alternative<PhaseCommand::Push>(command.value()));
}

TEST_CASE("TitlePhase::update - clicking quit returns Exit") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);

  const auto pos = registry.get<DrawPos>(FindButton(registry, U"おわる")).pos;
  const auto command = phase.update(
      registry, WithInput(InputState{.pointerPos = pos, .clickDown = true})
  );

  REQUIRE(std::holds_alternative<PhaseCommand::Exit>(command.value()));
}

TEST_CASE(
    "TitlePhase::onBeforePop - destroys the background, the logo and the menu"
) {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);

  phase.onBeforePop(registry);

  REQUIRE(registry.view<DrawPos>().empty());
  REQUIRE(registry.view<Menu>().empty());
}

#endif
