#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/DrawPos.hpp"
#include "Component/WorldPos.hpp"
#include "Screen.hpp"
#include "System/ProjectionSystem.hpp"

TEST_CASE("ProjectionSystem - writes screen position and depth") {
  entt::registry registry;
  const WorldPos world{.w = 120.0, .h = 30.0, .d = 80.0};
  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, world);
  registry.emplace<DrawPos>(entity);

  ProjectionSystem::Update(registry);

  const auto& drawPos = registry.get<DrawPos>(entity);
  REQUIRE(drawPos.pos == WorldToScreen(world));
  REQUIRE(drawPos.depth == 80.0);
}

TEST_CASE("ProjectionSystem - follows the latest world position") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, WorldPos{.w = 10.0, .d = 20.0});
  registry.emplace<DrawPos>(entity);
  ProjectionSystem::Update(registry);

  const WorldPos moved{.w = -50.0, .h = 5.0, .d = 300.0};
  registry.replace<WorldPos>(entity, moved);
  ProjectionSystem::Update(registry);

  const auto& drawPos = registry.get<DrawPos>(entity);
  REQUIRE(drawPos.pos == WorldToScreen(moved));
  REQUIRE(drawPos.depth == 300.0);
}

TEST_CASE("ProjectionSystem - leaves layer untouched") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, WorldPos{.w = 1.0, .d = 2.0});
  registry.emplace<DrawPos>(entity, DrawPos{.layer = DrawLayer::kWorldOverlay});

  ProjectionSystem::Update(registry);

  REQUIRE(registry.get<DrawPos>(entity).layer == DrawLayer::kWorldOverlay);
}

TEST_CASE("ProjectionSystem - does not rewrite entities without WorldPos") {
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<DrawPos>(
      entity,
      DrawPos{.pos = Vec2{16.0, 40.0}, .layer = DrawLayer::kHud, .depth = 7.0}
  );

  ProjectionSystem::Update(registry);

  const auto& drawPos = registry.get<DrawPos>(entity);
  REQUIRE(drawPos.pos == Vec2{16.0, 40.0});
  REQUIRE(drawPos.depth == 7.0);
}

#endif
