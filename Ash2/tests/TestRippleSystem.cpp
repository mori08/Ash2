#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"
#include "Component/Ripple.hpp"
#include "Config/TitleConfig.hpp"
#include "System/RippleSystem.hpp"

namespace {

const RippleConfig kConfig{
    .color = ColorF{0.1, 0.2, 0.3},
    .life = 4.0,
    .startRadius = 10.0,
    .endRadius = 100.0,
    .aspect = 0.3,
    .thickness = 2.0,
    .farScale = 0.5,
};

/// @brief 水面（0）・画面の下端（1）で割れた泡の知らせから波紋を作る
/// @note registry に波紋がまだ無いときだけ使う（2つ目以降は view から取れない）
entt::entity SpawnAt(entt::registry& registry, double depthRate) {
  RippleSystem::Spawn(
      registry, {BubblePop{.pos = Vec2{50.0, 200.0}, .depthRate = depthRate}},
      kConfig
  );
  return registry.view<Ripple>().back();
}

double RadiusX(const entt::registry& registry, entt::entity entity) {
  return std::get<EllipseFrameDrawable>(registry.get<Drawable>(entity))
      .radius.x;
}

}  // namespace

TEST_CASE("RippleSystem::Spawn - creates one ripple per pop") {
  entt::registry registry;
  RippleSystem::Spawn(
      registry,
      {BubblePop{.pos = Vec2{10.0, 200.0}, .depthRate = 0.2},
       BubblePop{.pos = Vec2{20.0, 300.0}, .depthRate = 0.8}},
      kConfig
  );

  REQUIRE(registry.view<Ripple>().size() == 2);
}

TEST_CASE("RippleSystem::Spawn - creates nothing without pops") {
  entt::registry registry;
  RippleSystem::Spawn(registry, {}, kConfig);

  REQUIRE(registry.view<Ripple>().empty());
}

TEST_CASE("RippleSystem::Spawn - starts opaque at the pop position") {
  entt::registry registry;
  const auto ripple = SpawnAt(registry, 1.0);

  REQUIRE(registry.get<DrawPos>(ripple).pos == Vec2{50.0, 200.0});
  REQUIRE(registry.get<DrawPos>(ripple).layer == DrawLayer::kBackground);
  REQUIRE(registry.get<DrawPos>(ripple).depth == -200.0);
  REQUIRE(registry.get<DrawColor>(ripple).color == ColorF{0.1, 0.2, 0.3, 1.0});
  const auto& drawable =
      std::get<EllipseFrameDrawable>(registry.get<Drawable>(ripple));
  REQUIRE(drawable.radius.x == Approx(10.0));
  REQUIRE(drawable.radius.y == Approx(3.0));
  REQUIRE(drawable.thickness == 2.0);
}

TEST_CASE("RippleSystem::Spawn - far ripples are smaller by far scale") {
  entt::registry farRegistry;
  entt::registry nearRegistry;
  const auto far = SpawnAt(farRegistry, 0.0);
  const auto near = SpawnAt(nearRegistry, 1.0);

  REQUIRE(
      RadiusX(farRegistry, far) / RadiusX(nearRegistry, near) == Approx(0.5)
  );
  // 広がった後の大きさも同じ比になる
  RippleSystem::Update(farRegistry, 2.0);
  RippleSystem::Update(nearRegistry, 2.0);
  REQUIRE(
      RadiusX(farRegistry, far) / RadiusX(nearRegistry, near) == Approx(0.5)
  );
}

TEST_CASE("RippleSystem::Update - radius grows monotonically") {
  entt::registry registry;
  const auto ripple = SpawnAt(registry, 1.0);

  double previous = RadiusX(registry, ripple);
  for (int32 i = 0; i < 39; ++i) {
    RippleSystem::Update(registry, 0.1);
    const double radius = RadiusX(registry, ripple);
    REQUIRE(radius > previous);
    previous = radius;
  }
  REQUIRE(previous < 100.0);
}

TEST_CASE("RippleSystem::Update - radius keeps the aspect ratio") {
  entt::registry registry;
  const auto ripple = SpawnAt(registry, 1.0);

  RippleSystem::Update(registry, 1.0);

  const auto& radius =
      std::get<EllipseFrameDrawable>(registry.get<Drawable>(ripple)).radius;
  REQUIRE(radius.y == Approx(radius.x * 0.3));
}

TEST_CASE("RippleSystem::Update - opacity fades linearly") {
  entt::registry registry;
  const auto ripple = SpawnAt(registry, 1.0);

  RippleSystem::Update(registry, 1.0);
  REQUIRE(registry.get<DrawColor>(ripple).color.a == Approx(0.75));

  RippleSystem::Update(registry, 2.0);
  REQUIRE(registry.get<DrawColor>(ripple).color.a == Approx(0.25));
}

TEST_CASE("RippleSystem::Update - ripple is destroyed at the end of its life") {
  entt::registry registry;
  const auto ripple = SpawnAt(registry, 1.0);

  RippleSystem::Update(registry, 3.9);
  REQUIRE(registry.valid(ripple));

  RippleSystem::Update(registry, 0.2);
  REQUIRE_FALSE(registry.valid(ripple));
  REQUIRE(registry.view<Ripple>().empty());
}

#endif
