#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Boundary.hpp"
#include "Component/Collider.hpp"
#include "Component/Hitstop.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/StageConfig.hpp"
#include "System/BoundarySystem.hpp"

namespace {

constexpr double kHalfW = 380.0;
constexpr double kHalfD = 280.0;
constexpr double kRadius = 24.0;

/// @brief StageConfig を registry.ctx() へ登録する
void SetupStage(entt::registry& registry) {
  registry.ctx().emplace<StageConfig>(
      StageConfig{.halfW = kHalfW, .halfD = kHalfD}
  );
}

/// @brief 境界クランプ対象のエンティティを生成する
/// @param pos 初期位置
/// @param vel 初期速度
entt::entity MakeBounded(
    entt::registry& registry, const WorldPos& pos, const Velocity& vel = {}
) {
  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, pos);
  registry.emplace<Velocity>(entity, vel);
  registry.emplace<Boundary>(entity);
  registry.emplace<Collider>(
      entity,
      Collider{
          .segmentStart = Vec3{0.0, 0.0, 0.0},
          .segmentEnd = Vec3{0.0, 0.0, 0.0},
          .radius = kRadius
      }
  );
  return entity;
}

}  // namespace

TEST_CASE("BoundarySystem - clamps out-of-range w to half minus radius") {
  entt::registry registry;
  SetupStage(registry);
  const auto entity =
      MakeBounded(registry, WorldPos{.w = 1000.0, .h = 10.0, .d = 0.0});

  BoundarySystem::Update(registry);

  REQUIRE(registry.get<WorldPos>(entity).w == Approx(kHalfW - kRadius));
}

TEST_CASE("BoundarySystem - clamps out-of-range d to half minus radius") {
  entt::registry registry;
  SetupStage(registry);
  const auto entity =
      MakeBounded(registry, WorldPos{.w = 0.0, .h = 10.0, .d = -1000.0});

  BoundarySystem::Update(registry);

  REQUIRE(registry.get<WorldPos>(entity).d == Approx(-(kHalfD - kRadius)));
}

TEST_CASE("BoundarySystem - leaves an in-range position untouched") {
  entt::registry registry;
  SetupStage(registry);
  const auto entity =
      MakeBounded(registry, WorldPos{.w = 50.0, .h = 10.0, .d = -30.0});

  BoundarySystem::Update(registry);

  const auto& pos = registry.get<WorldPos>(entity);
  REQUIRE(pos.w == Approx(50.0));
  REQUIRE(pos.d == Approx(-30.0));
}

TEST_CASE("BoundarySystem - does not touch h or Velocity") {
  entt::registry registry;
  SetupStage(registry);
  const auto entity = MakeBounded(
      registry, WorldPos{.w = 1000.0, .h = 42.0, .d = 0.0},
      Velocity{.w = 5.0, .h = 6.0, .d = 7.0}
  );

  BoundarySystem::Update(registry);

  const auto& pos = registry.get<WorldPos>(entity);
  const auto& vel = registry.get<Velocity>(entity);
  REQUIRE(pos.h == Approx(42.0));
  REQUIRE(vel.w == Approx(5.0));
  REQUIRE(vel.h == Approx(6.0));
  REQUIRE(vel.d == Approx(7.0));
}

TEST_CASE("BoundarySystem - leaves entities without Boundary untouched") {
  // 弾（Projectile）を想定。Boundary を持たないビューには入らない
  entt::registry registry;
  SetupStage(registry);
  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, WorldPos{.w = 1000.0});
  registry.emplace<Velocity>(entity);
  registry.emplace<Collider>(
      entity,
      Collider{
          .segmentStart = Vec3{0.0, 0.0, 0.0},
          .segmentEnd = Vec3{0.0, 0.0, 0.0},
          .radius = kRadius
      }
  );

  BoundarySystem::Update(registry);

  REQUIRE(registry.get<WorldPos>(entity).w == Approx(1000.0));
}

TEST_CASE("BoundarySystem - leaves entities without Collider untouched") {
  // 撃破後（Collider が外れたエンティティ）を想定
  entt::registry registry;
  SetupStage(registry);
  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, WorldPos{.w = 1000.0});
  registry.emplace<Velocity>(entity);
  registry.emplace<Boundary>(entity);

  BoundarySystem::Update(registry);

  REQUIRE(registry.get<WorldPos>(entity).w == Approx(1000.0));
}

TEST_CASE("BoundarySystem - clamps even while in hitstop") {
  entt::registry registry;
  SetupStage(registry);
  const auto entity =
      MakeBounded(registry, WorldPos{.w = 1000.0, .h = 10.0, .d = 0.0});
  registry.emplace<Hitstop>(entity, Hitstop{.remaining = 0.1});

  BoundarySystem::Update(registry);

  REQUIRE(registry.get<WorldPos>(entity).w == Approx(kHalfW - kRadius));
}

#endif
