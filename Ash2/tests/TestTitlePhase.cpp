#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Bubble.hpp"
#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"
#include "Component/FocusColor.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Menu.hpp"
#include "Component/Ripple.hpp"
#include "Config/TitleConfig.hpp"
#include "FrameData.hpp"
#include "Phase/TitlePhase.hpp"
#include "System/BubbleSystem.hpp"
#include "System/RippleSystem.hpp"

namespace {

const ColorF kBackgroundColor{0.9};
const ColorF kSurfaceLineColor{0.1};
const ColorF kPlantColor{0.3};
const ColorF kButtonNormalColor{0.5};
const ColorF kButtonFocusedColor{0.1, 0.2, 0.3};
constexpr double kBubbleInterval = 1.0;

/// @brief TitleConfig を ctx に積んだ registry で TitlePhase を push 済みにする
void PushTitle(entt::registry& registry, TitlePhase& phase) {
  registry.ctx().emplace<TitleConfig>(TitleConfig{
      .logoY = 160.0,
      .menuY = 440.0,
      .menuSpacing = 60.0,
      .buttonWidth = 200.0,
      .buttonHeight = 48.0,
      .backgroundColor = kBackgroundColor,
      .surfaceLineColor = kSurfaceLineColor,
      .logoColor = ColorF{0.2},
      .buttonNormalColor = kButtonNormalColor,
      .buttonFocusedColor = kButtonFocusedColor,
      .surfaceY = 360.0,
      .surfaceLineWidth = 1.5,
      .plantColor = kPlantColor,
      .plantStemWidth = 3.0,
      .plantSpreadAmplitude = 0.45,
      .plantStems = {{.dx = -9.0, .height = 36.0}, {.dx = 9.0, .height = 28.0}},
      .plants = {{.x = 90.0, .period = 8.0, .offset = 0.25}},
      // 泡が途中で割れて数が変わらないよう、昇る距離を大きく速さを小さくする
      .bubble =
          {
              .color = ColorF{0.6},
              .interval = kBubbleInterval,
              .intervalJitter = 0.5,
              .popMargin = 5.0,
              .farScale = 0.35,
              .popAlpha = 0.8,
              .rise = 1.0e6,
              .speed = 1.0,
              .radius = 5.0,
          },
      .ripple = {
          .color = ColorF{0.4},
          .life = 3.6,
          .startRadius = 6.0,
          .endRadius = 80.0,
          .aspect = 0.3,
          .thickness = 2.0,
          .farScale = 0.35,
      },
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
    const auto color = registry.get<const DrawColor>(entity).color;
    if (color == kBackgroundColor) {
      background = drawPos;
    } else if (color == kSurfaceLineColor) {
      surfaceLine = drawPos;
    }
  }

  REQUIRE(background.has_value());
  REQUIRE(surfaceLine.has_value());
  REQUIRE(background.value().depth > surfaceLine.value().depth);
}

TEST_CASE(
    "TitlePhase::onAfterPush - draws plants between background and surface line"
) {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);

  size_t stemCount = 0;
  for (const auto entity :
       registry.view<const DrawPos, const Drawable, const DrawColor>()) {
    if (registry.get<const DrawColor>(entity).color != kPlantColor) continue;
    ++stemCount;
    const auto& drawPos = registry.get<const DrawPos>(entity);
    REQUIRE(drawPos.layer == DrawLayer::kBackground);
    REQUIRE(drawPos.depth > 0.0);
    REQUIRE(drawPos.depth < 1.0);
  }
  REQUIRE(stemCount == 2);
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

TEST_CASE("TitlePhase::update - spawns one bubble on the first update") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);
  REQUIRE(registry.view<Bubble>().empty());

  (void)phase.update(registry, WithInput(InputState{}));

  REQUIRE(registry.view<Bubble>().size() == 1);
}

TEST_CASE("TitlePhase::update - spawns the next bubble within the jitter") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);
  // 時間を進めずに最初の1つを出し、次までの間隔だけを測る
  (void)phase.update(registry, FrameData{.dt = 0.0});
  REQUIRE(registry.view<Bubble>().size() == 1);

  // 間隔の 0.5 倍より前は出ない
  constexpr double kStep = kBubbleInterval / 100.0;
  for (int32 i = 0; i < 49; ++i) {
    (void)phase.update(registry, FrameData{.dt = kStep});
  }
  REQUIRE(registry.view<Bubble>().size() == 1);

  // 間隔の 1.5 倍までには出る
  for (int32 i = 49; i < 151; ++i) {
    (void)phase.update(registry, FrameData{.dt = kStep});
  }
  REQUIRE(registry.view<Bubble>().size() >= 2);
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

TEST_CASE("TitlePhase::onBeforePop - destroys the bubbles") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);
  (void)phase.update(registry, WithInput(InputState{}));
  REQUIRE_FALSE(registry.view<Bubble>().empty());

  phase.onBeforePop(registry);

  REQUIRE(registry.view<Bubble>().empty());
  REQUIRE(registry.view<DrawPos>().empty());
}

TEST_CASE("TitlePhase::onBeforePop - destroys the ripples") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);
  RippleSystem::Spawn(
      registry, {BubblePop{.pos = Vec2{100.0, 400.0}, .depthRate = 0.5}},
      registry.ctx().get<TitleConfig>().ripple
  );
  REQUIRE_FALSE(registry.view<Ripple>().empty());

  phase.onBeforePop(registry);

  REQUIRE(registry.view<Ripple>().empty());
  REQUIRE(registry.view<DrawPos>().empty());
}

#endif
