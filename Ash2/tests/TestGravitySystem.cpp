#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Gravity.hpp"
#include "Component/Hitstop.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "System/GravitySystem.hpp"

namespace {

constexpr double kAccel = 1000.0;

/// @brief 重力を受けるエンティティを生成する
/// @param h 初期の高さ（0 が地面）
/// @param velH 初期の垂直速度
entt::entity MakeFaller(entt::registry& registry, double h, double velH = 0.0) {
  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, WorldPos{.h = h});
  registry.emplace<Velocity>(entity, Velocity{.h = velH});
  registry.emplace<Gravity>(entity, Gravity{.accel = kAccel});
  return entity;
}

}  // namespace

TEST_CASE("GravitySystem - accelerates downward while airborne") {
  // 加速は速度にのみ効く。位置の更新は MovementSystem の担当
  entt::registry registry;
  const auto entity = MakeFaller(registry, 100.0);

  GravitySystem::Update(registry, 0.5);

  REQUIRE(registry.get<Velocity>(entity).h == Approx(-500.0));
  REQUIRE(registry.get<WorldPos>(entity).h == Approx(100.0));

  GravitySystem::Update(registry, 0.5);

  REQUIRE(registry.get<Velocity>(entity).h == Approx(-1000.0));
}

TEST_CASE("GravitySystem - clamps a sunken entity onto the ground") {
  // 地面を割り込んだ位置は 0 に戻し、垂直速度も落とす
  entt::registry registry;
  const auto entity = MakeFaller(registry, -5.0, -300.0);

  GravitySystem::Update(registry, 0.1);

  REQUIRE(registry.get<WorldPos>(entity).h == Approx(0.0));
  // 加速後の -400.0 ではなく 0。クランプは同じ呼び出しの中で後に効く
  REQUIRE(registry.get<Velocity>(entity).h == Approx(0.0));
}

TEST_CASE("GravitySystem - keeps upward velocity at ground level") {
  // ジャンプした瞬間は接地したまま上向きの速度を持つ。ここで速度を
  // 打ち消すとジャンプが成立しなくなる（クランプ条件が h < 0 である理由）
  entt::registry registry;
  const auto entity = MakeFaller(registry, 0.0, 300.0);

  GravitySystem::Update(registry, 0.1);

  REQUIRE(registry.get<Velocity>(entity).h == Approx(200.0));
  REQUIRE(registry.get<WorldPos>(entity).h == Approx(0.0));
}

TEST_CASE("GravitySystem - skips entities in hitstop") {
  entt::registry registry;
  const auto entity = MakeFaller(registry, 100.0, -50.0);
  registry.emplace<Hitstop>(entity, Hitstop{.remaining = 0.1});

  GravitySystem::Update(registry, 0.1);

  REQUIRE(registry.get<Velocity>(entity).h == Approx(-50.0));
  REQUIRE(registry.get<WorldPos>(entity).h == Approx(100.0));
}

TEST_CASE("GravitySystem - leaves entities without Gravity untouched") {
  // 重力は Gravity の付与で選択する（種別による分岐を持たない）
  entt::registry registry;
  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, WorldPos{.h = -5.0});
  registry.emplace<Velocity>(entity, Velocity{.h = -300.0});

  GravitySystem::Update(registry, 0.1);

  REQUIRE(registry.get<Velocity>(entity).h == Approx(-300.0));
  REQUIRE(registry.get<WorldPos>(entity).h == Approx(-5.0));
}

#endif
