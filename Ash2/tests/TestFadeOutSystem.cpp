#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/FadeOut.hpp"
#include "System/FadeOutSystem.hpp"

TEST_CASE("FadeOutSystem - decreases DrawColor::color.a with remaining time") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<FadeOut>(entity, FadeOut{.duration = 0.5, .remaining = 0.5});

  FadeOutSystem::Update(registry, 0.2);

  REQUIRE(registry.get<DrawColor>(entity).color.a == Approx(0.6));
}

TEST_CASE("FadeOutSystem - destroys entity once remaining reaches zero") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<FadeOut>(entity, FadeOut{.duration = 0.5, .remaining = 0.1});

  FadeOutSystem::Update(registry, 0.1);

  REQUIRE_FALSE(registry.valid(entity));
}

TEST_CASE("FadeOutSystem - keeps entity while remaining time is left") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<FadeOut>(entity, FadeOut{.duration = 0.5, .remaining = 0.3});

  FadeOutSystem::Update(registry, 0.1);

  REQUIRE(registry.valid(entity));
}

TEST_CASE("FadeOutSystem - emplaces DrawColor when entity has none") {
  // 未所持のエンティティでも白からフェードが始まる
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<FadeOut>(entity, FadeOut{.duration = 1.0, .remaining = 1.0});

  REQUIRE_FALSE(registry.all_of<DrawColor>(entity));

  FadeOutSystem::Update(registry, 0.25);

  REQUIRE(registry.all_of<DrawColor>(entity));
  REQUIRE(registry.get<DrawColor>(entity).color.a == Approx(0.75));
}

TEST_CASE("FadeOutSystem - preserves existing RGB while updating alpha") {
  // emplace_or_replace ではなく get_or_emplace で確保するため、既存の RGB
  // は白へ戻らない
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<DrawColor>(entity,
                              DrawColor{.color = ColorF{0.2, 0.5, 0.9}});
  registry.emplace<FadeOut>(entity, FadeOut{.duration = 1.0, .remaining = 1.0});

  FadeOutSystem::Update(registry, 0.5);

  const auto& color = registry.get<DrawColor>(entity).color;
  REQUIRE(color.r == Approx(0.2));
  REQUIRE(color.g == Approx(0.5));
  REQUIRE(color.b == Approx(0.9));
  REQUIRE(color.a == Approx(0.5));
}

TEST_CASE("FadeOutSystem - treats non-positive duration as zero opacity") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<FadeOut>(entity, FadeOut{.duration = 0.0, .remaining = 0.5});

  FadeOutSystem::Update(registry, 0.0);

  REQUIRE(registry.get<DrawColor>(entity).color.a == Approx(0.0));
}

TEST_CASE("FadeOutSystem - does not touch entities without Drawable") {
  // Drawable の variant を一切知らないため、持たないエンティティでも落ちない
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<FadeOut>(entity, FadeOut{.duration = 0.5, .remaining = 0.5});

  REQUIRE_FALSE(registry.all_of<Drawable>(entity));
  FadeOutSystem::Update(registry, 0.1);

  REQUIRE(registry.valid(entity));
  REQUIRE_FALSE(registry.all_of<Drawable>(entity));
}

TEST_CASE("FadeOutSystem - leaves entities without FadeOut untouched") {
  entt::registry registry;
  const auto entity = registry.create();

  FadeOutSystem::Update(registry, 0.1);

  REQUIRE(registry.valid(entity));
  REQUIRE_FALSE(registry.all_of<DrawColor>(entity));
}

#endif
