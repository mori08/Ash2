#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Menu.hpp"
#include "Factory/MenuFactory.hpp"
#include "System/MenuSystem.hpp"

namespace {

/// 上下2ボタンのメニュー。upper は (100, 100)、lower は (100, 200)、大きさは
/// 100x40
struct TestMenu {
  entt::entity menu;
  entt::entity upper;
  entt::entity lower;
};

constexpr Vec2 kUpperPos{100.0, 100.0};
constexpr Vec2 kLowerPos{100.0, 200.0};
constexpr Vec2 kNowhere{500.0, 500.0};

/// @brief 上のボタンを既定とするメニューを作る
/// @note upper の down は lower、lower の up は upper。lower の down は null
TestMenu MakeMenu(entt::registry& registry) {
  const TestMenu m{
      .menu = registry.create(),
      .upper = registry.create(),
      .lower = registry.create(),
  };
  MenuFactory::SetupMenu(registry, m.menu, m.upper);
  MenuFactory::SetupButton(
      registry, m.upper, m.menu,
      {.pos = kUpperPos, .size = SizeF{100.0, 40.0}, .down = m.lower}
  );
  MenuFactory::SetupButton(
      registry, m.lower, m.menu,
      {.pos = kLowerPos, .size = SizeF{100.0, 40.0}, .up = m.upper}
  );
  return m;
}

InputState Moved(Vec2 pos) {
  return InputState{.pointerPos = pos, .pointerMoved = pos};
}

bool IsFocused(const entt::registry& registry, entt::entity entity) {
  return registry.all_of<Focused>(entity);
}

}  // namespace

TEST_CASE("MenuSystem - cursor moved onto a button focuses it") {
  entt::registry registry;
  const auto m = MakeMenu(registry);

  const auto pressed = MenuSystem::Update(registry, Moved(kLowerPos), m.menu);

  REQUIRE_FALSE(pressed.has_value());
  REQUIRE(IsFocused(registry, m.lower));
  REQUIRE_FALSE(IsFocused(registry, m.upper));
}

TEST_CASE("MenuSystem - cursor moved onto empty space clears focus") {
  entt::registry registry;
  const auto m = MakeMenu(registry);
  (void)MenuSystem::Update(registry, Moved(kUpperPos), m.menu);

  (void)MenuSystem::Update(registry, Moved(kNowhere), m.menu);

  REQUIRE_FALSE(IsFocused(registry, m.upper));
  REQUIRE_FALSE(IsFocused(registry, m.lower));
}

TEST_CASE("MenuSystem - a still cursor does not take back the focus") {
  entt::registry registry;
  const auto m = MakeMenu(registry);
  (void)MenuSystem::Update(registry, Moved(kUpperPos), m.menu);
  // パッドで下へ移す
  (void)MenuSystem::Update(
      registry, InputState{.navigate = NavDirection::Down}, m.menu
  );
  REQUIRE(IsFocused(registry, m.lower));

  // カーソルは上のボタンの上で止まっている（pointerMoved は none）
  (void)MenuSystem::Update(
      registry, InputState{.pointerPos = kUpperPos}, m.menu
  );

  REQUIRE(IsFocused(registry, m.lower));
  REQUIRE_FALSE(IsFocused(registry, m.upper));
}

TEST_CASE(
    "MenuSystem - navigate without focus selects the default button only"
) {
  entt::registry registry;
  const auto m = MakeMenu(registry);

  (void)MenuSystem::Update(
      registry, InputState{.navigate = NavDirection::Down}, m.menu
  );

  REQUIRE(IsFocused(registry, m.upper));
  REQUIRE_FALSE(IsFocused(registry, m.lower));
}

TEST_CASE("MenuSystem - navigate with focus moves to the destination") {
  entt::registry registry;
  const auto m = MakeMenu(registry);
  (void)MenuSystem::Update(registry, Moved(kUpperPos), m.menu);

  (void)MenuSystem::Update(
      registry, InputState{.navigate = NavDirection::Down}, m.menu
  );

  REQUIRE(IsFocused(registry, m.lower));
  REQUIRE_FALSE(IsFocused(registry, m.upper));
}

TEST_CASE("MenuSystem - navigate toward a null destination keeps the focus") {
  entt::registry registry;
  const auto m = MakeMenu(registry);
  (void)MenuSystem::Update(registry, Moved(kLowerPos), m.menu);

  (void)MenuSystem::Update(
      registry, InputState{.navigate = NavDirection::Down}, m.menu
  );

  REQUIRE(IsFocused(registry, m.lower));
}

TEST_CASE("MenuSystem - confirm without focus returns none") {
  entt::registry registry;
  const auto m = MakeMenu(registry);

  const auto pressed =
      MenuSystem::Update(registry, InputState{.confirmDown = true}, m.menu);

  REQUIRE_FALSE(pressed.has_value());
}

TEST_CASE("MenuSystem - confirm with focus returns the focused button") {
  entt::registry registry;
  const auto m = MakeMenu(registry);
  (void)MenuSystem::Update(registry, Moved(kLowerPos), m.menu);

  const auto pressed =
      MenuSystem::Update(registry, InputState{.confirmDown = true}, m.menu);

  REQUIRE(pressed.has_value());
  REQUIRE(*pressed == m.lower);
}

TEST_CASE(
    "MenuSystem - click returns the button under the pointer and focuses it"
) {
  entt::registry registry;
  const auto m = MakeMenu(registry);
  // パッドで上を選んだ後、下のボタンをクリックする
  (void)MenuSystem::Update(
      registry, InputState{.navigate = NavDirection::Down}, m.menu
  );

  const auto pressed = MenuSystem::Update(
      registry, InputState{.pointerPos = kLowerPos, .clickDown = true}, m.menu
  );

  REQUIRE(pressed.has_value());
  REQUIRE(*pressed == m.lower);
  REQUIRE(IsFocused(registry, m.lower));
  REQUIRE_FALSE(IsFocused(registry, m.upper));
}

TEST_CASE("MenuSystem - click on empty space returns none") {
  entt::registry registry;
  const auto m = MakeMenu(registry);

  const auto pressed = MenuSystem::Update(
      registry, InputState{.pointerPos = kNowhere, .clickDown = true}, m.menu
  );

  REQUIRE_FALSE(pressed.has_value());
}

TEST_CASE("MenuSystem - move and click in the same frame updates focus first") {
  entt::registry registry;
  const auto m = MakeMenu(registry);

  const auto pressed = MenuSystem::Update(
      registry,
      InputState{
          .pointerPos = kLowerPos,
          .clickDown = true,
          .pointerMoved = kLowerPos,
      },
      m.menu
  );

  REQUIRE(pressed.has_value());
  REQUIRE(*pressed == m.lower);
  REQUIRE(IsFocused(registry, m.lower));
}

TEST_CASE("MenuSystem - does not touch Focused of another menu") {
  entt::registry registry;
  const auto a = MakeMenu(registry);
  const auto b = MakeMenu(registry);
  (void)MenuSystem::Update(registry, Moved(kUpperPos), b.menu);
  REQUIRE(IsFocused(registry, b.upper));

  (void)MenuSystem::Update(registry, Moved(kLowerPos), a.menu);

  REQUIRE(IsFocused(registry, a.lower));
  REQUIRE(IsFocused(registry, b.upper));
}

#endif
