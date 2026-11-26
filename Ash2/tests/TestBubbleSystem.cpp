#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Bubble.hpp"
#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"
#include "Config/TitleConfig.hpp"
#include "FrameData.hpp"
#include "System/BubbleSystem.hpp"

namespace {

constexpr int32 kSpawnCount = 200;
const RectF kArea{0.0, 100.0, 400.0, 300.0};
const BubbleConfig kConfig{
    .color = ColorF{0.1, 0.2, 0.3},
    .interval = 1.0,
    .intervalJitter = 0.5,
    .popMargin = 5.0,
    .farScale = 0.35,
    .popAlpha = 0.8,
    .rise = 100.0,
    .speed = 20.0,
    .radius = 5.0,
};

/// @brief 泡を kSpawnCount 個作る
void SpawnMany(entt::registry& registry) {
  for (int32 i = 0; i < kSpawnCount; ++i) {
    BubbleSystem::Spawn(registry, FrameData{}, kConfig, kArea);
  }
}

/// @brief 昇る途中の泡を作る（200 で生まれ、100 で割れ、毎秒 10 昇る）
/// @param y 現在の y 座標
/// @param depthRate 割れる位置の奥行き
entt::entity MakeBubble(
    entt::registry& registry, double y, double depthRate = 0.5
) {
  const auto entity = registry.create();
  registry.emplace<Bubble>(
      entity,
      Bubble{
          .popY = 100.0,
          .startY = 200.0,
          .depthRate = depthRate,
          .speed = 10.0,
          .popAlpha = 0.8
      }
  );
  registry.emplace<DrawPos>(entity, DrawPos{.pos = Vec2{50.0, y}});
  registry.emplace<DrawColor>(
      entity, DrawColor{.color = ColorF{0.1, 0.2, 0.3, 0.0}}
  );
  return entity;
}

}  // namespace

TEST_CASE("BubbleSystem::Spawn - bubbles pop inside the area") {
  entt::registry registry;
  SpawnMany(registry);

  REQUIRE(registry.view<Bubble>().size() == kSpawnCount);
  for (const auto [entity, bubble, drawPos] :
       registry.view<const Bubble, const DrawPos>().each()) {
    REQUIRE(drawPos.pos.x >= kArea.x);
    REQUIRE(drawPos.pos.x <= kArea.x + kArea.w);
    REQUIRE(bubble.popY >= kArea.y + kConfig.popMargin);
    REQUIRE(bubble.popY <= kArea.y + kArea.h);
  }
}

TEST_CASE("BubbleSystem::Spawn - farther bubbles rise less and slower") {
  entt::registry registry;
  SpawnMany(registry);

  for (const auto [entity, bubble, drawPos] :
       registry.view<const Bubble, const DrawPos>().each()) {
    // 水面（100）で 0.35、下端（400）で 1
    const double scale = 0.35 + 0.65 * (bubble.popY - 100.0) / 300.0;
    REQUIRE(drawPos.pos.y - bubble.popY == Approx(100.0 * scale));
    REQUIRE(bubble.speed == Approx(20.0 * scale));
  }
}

TEST_CASE("BubbleSystem::Spawn - bubbles are transparent and equal in size") {
  entt::registry registry;
  SpawnMany(registry);

  for (const auto [entity, bubble, drawable, drawColor] :
       registry.view<const Bubble, const Drawable, const DrawColor>().each()) {
    REQUIRE(std::get<CircleDrawable>(drawable).radius == kConfig.radius);
    REQUIRE(drawColor.color == ColorF{0.1, 0.2, 0.3, 0.0});
  }
}

TEST_CASE("BubbleSystem::Spawn - farther bubbles are drawn first") {
  entt::registry registry;
  SpawnMany(registry);

  for (const auto [entity, bubble, drawPos] :
       registry.view<const Bubble, const DrawPos>().each()) {
    REQUIRE(drawPos.layer == DrawLayer::kBackground);
    REQUIRE(drawPos.depth == -bubble.popY);
  }
}

TEST_CASE("BubbleSystem::Update - bubble rises by speed times dt") {
  entt::registry registry;
  const auto bubble = MakeBubble(registry, 200.0);

  (void)BubbleSystem::Update(registry, FrameData{.dt = 0.5});

  REQUIRE(registry.get<DrawPos>(bubble).pos.y == Approx(195.0));
  REQUIRE(registry.get<DrawPos>(bubble).pos.x == 50.0);
}

TEST_CASE("BubbleSystem::Update - bubble gets denser as it rises") {
  entt::registry registry;
  const auto bubble = MakeBubble(registry, 200.0);

  // 半分（200 → 150）昇ると 0.8 * 0.5^2
  (void)BubbleSystem::Update(registry, FrameData{.dt = 5.0});
  REQUIRE(registry.get<DrawColor>(bubble).color.a == Approx(0.2));

  // 割れる直前（150 → 101）は 0.8 * 0.99^2
  (void)BubbleSystem::Update(registry, FrameData{.dt = 4.9});
  REQUIRE(registry.get<DrawColor>(bubble).color.a == Approx(0.8 * 0.99 * 0.99));
}

TEST_CASE("BubbleSystem::Update - bubble is destroyed on reaching popY") {
  entt::registry registry;
  const auto reaching = MakeBubble(registry, 105.0);
  const auto passing = MakeBubble(registry, 101.0);

  // 105 - 5 = 100 は割れる位置に着いたので消える
  (void)BubbleSystem::Update(registry, FrameData{.dt = 0.5});

  REQUIRE_FALSE(registry.valid(reaching));
  REQUIRE_FALSE(registry.valid(passing));
  REQUIRE(registry.view<Bubble>().empty());
}

TEST_CASE("BubbleSystem::Update - bubble stays before reaching popY") {
  entt::registry registry;
  const auto bubble = MakeBubble(registry, 105.0);

  (void)BubbleSystem::Update(registry, FrameData{.dt = 0.4});

  REQUIRE(registry.valid(bubble));
  REQUIRE(registry.get<DrawPos>(bubble).pos.y == Approx(101.0));
}

TEST_CASE("BubbleSystem::Update - returns the pop position and depth rate") {
  entt::registry registry;
  (void)MakeBubble(registry, 105.0, 0.25);

  const auto pops = BubbleSystem::Update(registry, FrameData{.dt = 0.5});

  REQUIRE(pops.size() == 1);
  REQUIRE(pops[0].pos.x == 50.0);
  REQUIRE(pops[0].pos.y == 100.0);
  REQUIRE(pops[0].depthRate == 0.25);
}

TEST_CASE("BubbleSystem::Update - returns nothing while no bubble pops") {
  entt::registry registry;
  (void)MakeBubble(registry, 105.0);

  const auto pops = BubbleSystem::Update(registry, FrameData{.dt = 0.4});

  REQUIRE(pops.isEmpty());
}

TEST_CASE("BubbleSystem::Spawn - depth rate stays between 0 and 1") {
  entt::registry registry;
  SpawnMany(registry);

  for (const auto [entity, bubble] : registry.view<const Bubble>().each()) {
    REQUIRE(bubble.depthRate >= 0.0);
    REQUIRE(bubble.depthRate <= 1.0);
  }
}

#endif
