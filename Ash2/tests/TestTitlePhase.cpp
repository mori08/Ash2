#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/FocusColor.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Menu.hpp"
#include "Component/ScreenPos.hpp"
#include "Config/TitleConfig.hpp"
#include "FrameData.hpp"
#include "Phase/TitlePhase.hpp"

namespace {

/// @brief TitleConfig を ctx に積んだ registry で TitlePhase を push 済みにする
void PushTitle(entt::registry& registry, TitlePhase& phase) {
  registry.ctx().emplace<TitleConfig>(TitleConfig{
      .logoY = 160.0,
      .menuY = 440.0,
      .menuSpacing = 60.0,
      .buttonWidth = 200.0,
      .buttonHeight = 48.0,
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
  // 色の値は仮置きのため、FocusColor の設定どおりに反映されたかだけを見る
  REQUIRE(
      registry.get<DrawColor>(start).color ==
      registry.get<FocusColor>(start).focused
  );
  REQUIRE(
      registry.get<DrawColor>(quit).color ==
      registry.get<FocusColor>(quit).normal
  );
}

TEST_CASE("TitlePhase::update - clicking each button returns None") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);

  // 対応表に処理が登録されていなければ throw するため、両方押して確かめる
  for (const auto label : {U"はじめる"_sv, U"おわる"_sv}) {
    const auto pos = registry.get<ScreenPos>(FindButton(registry, label)).pos;
    const auto command = phase.update(
        registry, WithInput(InputState{.pointerPos = pos, .clickDown = true})
    );
    REQUIRE(std::holds_alternative<PhaseCommand::None>(command.value()));
  }
}

TEST_CASE("TitlePhase::onBeforePop - destroys the logo and the menu") {
  entt::registry registry;
  TitlePhase phase;
  PushTitle(registry, phase);

  phase.onBeforePop(registry);

  REQUIRE(registry.view<ScreenPos>().empty());
  REQUIRE(registry.view<Menu>().empty());
}

#endif
