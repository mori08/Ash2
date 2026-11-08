#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/DrawColor.hpp"
#include "Component/FocusColor.hpp"
#include "Component/Menu.hpp"
#include "System/FocusColorSystem.hpp"

namespace {
constexpr ColorF kNormal{0.2, 0.3, 0.4, 1.0};
constexpr ColorF kFocused{0.9, 0.8, 0.1, 1.0};

entt::entity MakeButton(entt::registry& registry) {
  const auto entity = registry.create();
  registry.emplace<FocusColor>(
      entity, FocusColor{.normal = kNormal, .focused = kFocused}
  );
  return entity;
}
}  // namespace

TEST_CASE("FocusColorSystem - focused entity gets the focused color") {
  entt::registry registry;
  const auto entity = MakeButton(registry);
  registry.emplace<Focused>(entity);

  FocusColorSystem::Update(registry);

  REQUIRE(registry.get<DrawColor>(entity).color == kFocused);
}

TEST_CASE("FocusColorSystem - unfocused entity gets the normal color") {
  entt::registry registry;
  const auto entity = MakeButton(registry);

  FocusColorSystem::Update(registry);

  REQUIRE(registry.get<DrawColor>(entity).color == kNormal);
}

TEST_CASE("FocusColorSystem - color follows the focus when it moves") {
  entt::registry registry;
  const auto first = MakeButton(registry);
  const auto second = MakeButton(registry);
  registry.emplace<Focused>(first);
  FocusColorSystem::Update(registry);

  registry.remove<Focused>(first);
  registry.emplace<Focused>(second);
  FocusColorSystem::Update(registry);

  REQUIRE(registry.get<DrawColor>(first).color == kNormal);
  REQUIRE(registry.get<DrawColor>(second).color == kFocused);
}

#endif
