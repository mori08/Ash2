#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/AttackState.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/NeutralState.hpp"
#include "Component/Player.hpp"
#include "Component/WorldPos.hpp"
#include "Phase/FrameData.hpp"
#include "System/AttackStateSystem.hpp"

TEST_CASE("AttackStateSystem - timer decreases but stays in AttackState") {
  // タイマーが残っている間は AttackState のまま、timer が減算される
  entt::registry registry;

  auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player);
  registry.emplace<AttackState>(player,
                                AttackState{.clip = U"attack", .timer = 1.0});

  const FrameData frameData{.dt = 0.5};
  AttackStateSystem::Update(registry, frameData);

  REQUIRE(registry.all_of<AttackState>(player));
  REQUIRE(registry.get<AttackState>(player).timer == Approx(0.5));
  REQUIRE_FALSE(registry.all_of<NeutralState>(player));
}

TEST_CASE("AttackStateSystem - timer expiry transitions Attack to Neutral") {
  // timer <= 0 になったら AttackState を外し NeutralState を付与する
  entt::registry registry;

  auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player);
  registry.emplace<AttackState>(player,
                                AttackState{.clip = U"attack", .timer = 0.1});

  const FrameData frameData{.dt = 0.5};
  AttackStateSystem::Update(registry, frameData);

  REQUIRE_FALSE(registry.all_of<AttackState>(player));
  REQUIRE(registry.all_of<NeutralState>(player));
}

TEST_CASE(
    "AttackStateSystem - destroys attack entity on Attack to Neutral "
    "transition") {
  // AttackState.entity が entt::null でない場合、終了時に
  // DestroyWithChildren で破棄される
  entt::registry registry;

  auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player);

  auto attackEntity = registry.create();
  registry.emplace<WorldPos>(attackEntity);
  Hierarchy::Attach(registry, player, attackEntity);

  registry.emplace<AttackState>(
      player,
      AttackState{.clip = U"attack", .timer = 0.1, .entity = attackEntity});

  const FrameData frameData{.dt = 0.5};
  AttackStateSystem::Update(registry, frameData);

  REQUIRE_FALSE(registry.valid(attackEntity));
  REQUIRE_FALSE(registry.all_of<AttackState>(player));
  REQUIRE(registry.all_of<NeutralState>(player));
}

TEST_CASE(
    "AttackStateSystem - ranged attack (entity == entt::null) transitions "
    "safely") {
  // 遠距離攻撃時（entity == entt::null）でも安全に遷移する
  entt::registry registry;

  auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player);
  registry.emplace<AttackState>(player, AttackState{.clip = U"ranged_attack",
                                                    .timer = 0.1,
                                                    .entity = entt::null});

  const FrameData frameData{.dt = 0.5};
  AttackStateSystem::Update(registry, frameData);

  REQUIRE_FALSE(registry.all_of<AttackState>(player));
  REQUIRE(registry.all_of<NeutralState>(player));
}

#endif
