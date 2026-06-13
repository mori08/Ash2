#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "System/MovementSystem.hpp"

TEST_CASE("MovementSystem - moves entity by velocity each frame") {
  // Velocity に従って WorldPos が更新される
  entt::registry registry;

  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Velocity>(entity, Velocity{.w = 100.0, .h = 0.0, .d = 0.0});

  MovementSystem::Update(registry, 0.5);

  REQUIRE(registry.get<WorldPos>(entity).w == Approx(50.0));

  MovementSystem::Update(registry, 0.5);

  REQUIRE(registry.get<WorldPos>(entity).w == Approx(100.0));
}

#endif
