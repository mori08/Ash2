#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Hitstop.hpp"
#include "System/HitstopSystem.hpp"

TEST_CASE("HitstopSystem - decrements remaining by dt") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<Hitstop>(entity, Hitstop{.remaining = 0.1});

  HitstopSystem::Update(registry, 0.03);

  REQUIRE(registry.get<Hitstop>(entity).remaining == Approx(0.07));
}

TEST_CASE("HitstopSystem - removes Hitstop once remaining reaches zero") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<Hitstop>(entity, Hitstop{.remaining = 0.05});

  HitstopSystem::Update(registry, 0.05);

  REQUIRE_FALSE(registry.all_of<Hitstop>(entity));
}

TEST_CASE("HitstopSystem - removes Hitstop once remaining goes negative") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<Hitstop>(entity, Hitstop{.remaining = 0.02});

  HitstopSystem::Update(registry, 0.05);

  REQUIRE_FALSE(registry.all_of<Hitstop>(entity));
}

TEST_CASE("HitstopSystem - leaves entities without Hitstop untouched") {
  entt::registry registry;
  const auto entity = registry.create();

  HitstopSystem::Update(registry, 0.05);

  REQUIRE_FALSE(registry.all_of<Hitstop>(entity));
}

TEST_CASE("HitstopSystem - updates multiple entities independently") {
  entt::registry registry;
  const auto shortEntity = registry.create();
  registry.emplace<Hitstop>(shortEntity, Hitstop{.remaining = 0.02});
  const auto longEntity = registry.create();
  registry.emplace<Hitstop>(longEntity, Hitstop{.remaining = 0.20});

  HitstopSystem::Update(registry, 0.05);

  REQUIRE_FALSE(registry.all_of<Hitstop>(shortEntity));
  REQUIRE(registry.get<Hitstop>(longEntity).remaining == Approx(0.15));
}

#endif
