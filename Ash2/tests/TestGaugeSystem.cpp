#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Drawable.hpp"
#include "Component/Gauge.hpp"
#include "Component/Hp.hpp"
#include "Component/Stamina.hpp"
#include "System/GaugeSystem.hpp"

namespace {

constexpr double kFullWidth = 200.0;

/// @brief GaugeT を持つゲージを生成する（幅 0 の RectDrawable）
template <typename GaugeT>
entt::entity MakeGauge(entt::registry& registry, entt::entity source) {
  const auto gauge = registry.create();
  registry.emplace<GaugeT>(gauge, GaugeT{.fullWidth = kFullWidth});
  registry.emplace<GaugeSource>(gauge, GaugeSource{.entity = source});
  registry.emplace<Drawable>(gauge, RectDrawable{.size = SizeF{0.0, 18.0}});
  return gauge;
}

/// @brief Hp を持つエンティティを生成する
entt::entity MakeHolder(entt::registry& registry, int32 max, int32 current) {
  const auto entity = registry.create();
  registry.emplace<Hp>(entity, Hp{.max = max, .current = current});
  registry.emplace<Stamina>(entity, Stamina{.max = max, .current = current});
  return entity;
}

/// @brief ゲージの現在の幅を返す
double WidthOf(const entt::registry& registry, entt::entity gauge) {
  return std::get<RectDrawable>(registry.get<Drawable>(gauge)).size.x;
}

}  // namespace

TEST_CASE("GaugeSystem - HpGauge width follows the hp ratio") {
  entt::registry registry;
  const auto holder = MakeHolder(registry, 100, 50);
  const auto gauge = MakeGauge<HpGauge>(registry, holder);

  GaugeSystem::Update(registry);

  REQUIRE(WidthOf(registry, gauge) == Approx(kFullWidth * 0.5));
}

TEST_CASE("GaugeSystem - StaminaGauge width follows the stamina ratio") {
  entt::registry registry;
  const auto holder = MakeHolder(registry, 100, 25);
  const auto gauge = MakeGauge<StaminaGauge>(registry, holder);

  GaugeSystem::Update(registry);

  REQUIRE(WidthOf(registry, gauge) == Approx(kFullWidth * 0.25));
}

TEST_CASE("GaugeSystem - ratio is clamped to the 0 to 1 range") {
  entt::registry registry;
  const auto over = MakeHolder(registry, 100, 150);
  const auto under = MakeHolder(registry, 100, -10);
  const auto overGauge = MakeGauge<HpGauge>(registry, over);
  const auto underGauge = MakeGauge<HpGauge>(registry, under);

  GaugeSystem::Update(registry);

  REQUIRE(WidthOf(registry, overGauge) == Approx(kFullWidth));
  REQUIRE(WidthOf(registry, underGauge) == Approx(0.0));
}

TEST_CASE("GaugeSystem - width is 0 when max is 0") {
  entt::registry registry;
  const auto holder = MakeHolder(registry, 0, 0);
  const auto gauge = MakeGauge<HpGauge>(registry, holder);

  GaugeSystem::Update(registry);

  REQUIRE(WidthOf(registry, gauge) == Approx(0.0));
}

TEST_CASE("GaugeSystem - each gauge reflects its own source") {
  entt::registry registry;
  const auto half = MakeHolder(registry, 100, 50);
  const auto full = MakeHolder(registry, 100, 100);
  const auto halfGauge = MakeGauge<HpGauge>(registry, half);
  const auto fullGauge = MakeGauge<HpGauge>(registry, full);

  GaugeSystem::Update(registry);

  REQUIRE(WidthOf(registry, halfGauge) == Approx(kFullWidth * 0.5));
  REQUIRE(WidthOf(registry, fullGauge) == Approx(kFullWidth));
}
#endif
