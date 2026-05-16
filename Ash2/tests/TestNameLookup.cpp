#if USE_TEST
#include <ThirdParty/Catch2/catch.hpp>
#include <ThirdParty/entt/entt.hpp>

#include "Component/Name.hpp"
#include "System/NameLookup.hpp"

TEST_CASE("NameLookup - auto-cleanup on entity destroy") {
  entt::registry registry;
  registry.ctx().emplace<NameLookup>();
  NameLookupSystem::Connect(registry);

  const auto entity = registry.create();
  registry.emplace<Name>(entity, Name{U"test"});
  registry.ctx().get<NameLookup>()[U"test"] = entity;

  REQUIRE(registry.ctx().get<NameLookup>().contains(U"test"));

  registry.destroy(entity);

  REQUIRE_FALSE(registry.ctx().get<NameLookup>().contains(U"test"));
}

TEST_CASE("NameLookup - auto-cleanup when Name component is removed") {
  entt::registry registry;
  registry.ctx().emplace<NameLookup>();
  NameLookupSystem::Connect(registry);

  const auto entity = registry.create();
  registry.emplace<Name>(entity, Name{U"test"});
  registry.ctx().get<NameLookup>()[U"test"] = entity;

  registry.remove<Name>(entity);

  REQUIRE_FALSE(registry.ctx().get<NameLookup>().contains(U"test"));

  registry.destroy(entity);
}
#endif
