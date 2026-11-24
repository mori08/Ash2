#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/DrawPos.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Plant.hpp"
#include "System/PlantSwaySystem.hpp"

namespace {

constexpr double kRootX = 100.0;
constexpr double kDx = 10.0;
constexpr double kAmplitude = 0.5;
constexpr double kPeriod = 8.0;

/// @brief 棒を1本持つ株を作り、子の棒を返す
entt::entity MakePlant(entt::registry& registry, double offset) {
  const auto plant = registry.create();
  registry.emplace<DrawPos>(plant, DrawPos{.pos = Vec2{kRootX, 0.0}});
  registry.emplace<Plant>(
      plant,
      Plant{.period = kPeriod, .offset = offset, .spreadAmplitude = kAmplitude}
  );
  const auto stem = registry.create();
  registry.emplace<PlantStem>(stem, PlantStem{.dx = kDx});
  registry.emplace<DrawPos>(stem, DrawPos{.pos = Vec2{kRootX, 0.0}});
  Hierarchy::Attach(registry, plant, stem);
  return stem;
}

}  // namespace

TEST_CASE("PlantSwaySystem::Update - spread peaks at a quarter period") {
  entt::registry registry;
  const auto stem = MakePlant(registry, 0.0);

  PlantSwaySystem::Update(registry, kPeriod / 4.0);

  REQUIRE(
      registry.get<DrawPos>(stem).pos.x ==
      Approx(kRootX + kDx * (1.0 + kAmplitude))
  );
}

TEST_CASE("PlantSwaySystem::Update - spread returns after one period") {
  entt::registry registry;
  const auto stem = MakePlant(registry, 0.0);

  PlantSwaySystem::Update(registry, kPeriod);

  REQUIRE(registry.get<DrawPos>(stem).pos.x == Approx(kRootX + kDx));
}

TEST_CASE("PlantSwaySystem::Update - offset shifts the start of the spread") {
  entt::registry registry;
  const auto stemA = MakePlant(registry, 0.0);
  const auto stemB = MakePlant(registry, 0.25);

  PlantSwaySystem::Update(registry, 0.0);

  REQUIRE(registry.get<DrawPos>(stemA).pos.x == Approx(kRootX + kDx));
  REQUIRE(
      registry.get<DrawPos>(stemB).pos.x ==
      Approx(kRootX + kDx * (1.0 + kAmplitude))
  );
}

#endif
