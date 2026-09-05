#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Collider.hpp"
#include "Component/Enemy.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/LockOn.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "FrameData.hpp"
#include "System/LockOnSystem.hpp"

namespace {

/// @brief テスト用のロック設定
const LockConfig kLockConfig{
    .capsuleScale = 2.0,
    .angleLimitDeg = 60.0,
    .angleWeight = 1.0,
    .stickTilt = 0.5,
    .stickRelease = 0.24,
    .lockedColor = ColorF{1.0, 0.0, 0.0},
    .lockedAlpha = 1.0,
    .warningAlpha = 0.4,
    .halfColor = ColorF{1.0, 1.0, 1.0},
    .halfAlpha = 0.6,
};

/// @brief 半径0（点扱い）のコライダー
const Collider kPointCollider{
    .segmentStart = {0.0, 0.0, 0.0},
    .segmentEnd = {0.0, 0.0, 0.0},
    .radius = 0.0
};

/// @brief テスト用の registry.ctx() セットアップ（PlayerConfig::lock のみ）
void SetupContext(entt::registry& registry) {
  registry.ctx().emplace<PlayerConfig>(PlayerConfig{.lock = kLockConfig});
}

/// @brief テスト用の敵エンティティを生成する
entt::entity MakeEnemy(
    entt::registry& registry, const WorldPos& pos, const Collider& col,
    const EnemyMotion::Variant& motion = EnemyMotion::Idle{}
) {
  const auto enemy = registry.create();
  registry.emplace<Enemy>(enemy);
  registry.emplace<WorldPos>(enemy, pos);
  registry.emplace<Collider>(enemy, col);
  registry.emplace<EnemyMotion::Variant>(enemy, motion);
  return enemy;
}

/// @brief テスト用のプレイヤーエンティティを生成する（LockOn 付き）
entt::entity MakePlayer(entt::registry& registry, const WorldPos& pos) {
  const auto player = registry.create();
  registry.emplace<WorldPos>(player, pos);
  registry.emplace<Collider>(player, kPointCollider);
  registry.emplace<LockOn>(player);
  return player;
}

}  // namespace

// ---- AimPoint ----

TEST_CASE(
    "LockOnSystem::AimPoint - returns the segment midpoint offset by pos"
) {
  const WorldPos pos{.w = 10.0, .h = 20.0, .d = 30.0};
  const Collider col{
      .segmentStart = {0.0, 0.0, 0.0},
      .segmentEnd = {0.0, 40.0, 0.0},
      .radius = 5.0
  };
  const auto aim = LockOnSystem::AimPoint(pos, col);
  REQUIRE(aim.w == Approx(10.0));
  REQUIRE(aim.h == Approx(40.0));
  REQUIRE(aim.d == Approx(30.0));
}

// ---- Project ----

TEST_CASE(
    "LockOnSystem::Project - projects the segment and scales the radius"
) {
  const WorldPos pos{.w = 0.0, .h = 0.0, .d = 0.0};
  const Collider col{
      .segmentStart = {0.0, 0.0, 0.0},
      .segmentEnd = {0.0, 40.0, 0.0},
      .radius = 10.0
  };
  const auto cap = LockOnSystem::Project(pos, col, 2.0);
  REQUIRE(cap.start == Vec2{0.0, 0.0});
  REQUIRE(cap.end == Vec2{0.0, -40.0});
  REQUIRE(cap.radius == Approx(20.0));
}

// ---- Contains ----

TEST_CASE("LockOnSystem::Contains - true within the capsule, false outside") {
  const ScreenCapsule cap{
      .start = {0.0, 0.0}, .end = {0.0, -40.0}, .radius = 10.0
  };
  REQUIRE(LockOnSystem::Contains(cap, Vec2{5.0, -20.0}));
  REQUIRE_FALSE(LockOnSystem::Contains(cap, Vec2{15.0, -20.0}));
}

TEST_CASE("LockOnSystem::Contains - handles a degenerate (point) capsule") {
  const ScreenCapsule cap{
      .start = {0.0, 0.0}, .end = {0.0, 0.0}, .radius = 10.0
  };
  REQUIRE(LockOnSystem::Contains(cap, Vec2{5.0, 5.0}));
  REQUIRE_FALSE(LockOnSystem::Contains(cap, Vec2{20.0, 0.0}));
}

// ---- SelectByDirection ----

TEST_CASE(
    "LockOnSystem::SelectByDirection - returns entt::null for a zero "
    "direction"
) {
  entt::registry registry;
  const auto axis = MakeEnemy(registry, WorldPos{}, kPointCollider);
  // 方向が非ゼロなら選ばれる候補を置く。これがないと、候補が1つもないから
  // null になったのか、ゼロ方向だから null になったのかを区別できない
  MakeEnemy(registry, WorldPos{.w = 0.0, .h = 0.0, .d = 50.0}, kPointCollider);
  REQUIRE(
      (LockOnSystem::SelectByDirection(
           registry, axis, Vec2::Zero(), kLockConfig
       ) == entt::null)
  );
}

TEST_CASE(
    "LockOnSystem::SelectByDirection - cycles to the next candidate when the "
    "axis moves to the selected enemy"
) {
  // 軸を選ばれた敵へ移すと次の敵が選ばれる（同じ方向へ倒すたびの順送り）
  entt::registry registry;
  const auto axis = MakeEnemy(registry, WorldPos{}, kPointCollider);
  const auto near = MakeEnemy(
      registry, WorldPos{.w = 0.0, .h = 0.0, .d = 50.0}, kPointCollider
  );
  const auto far = MakeEnemy(
      registry, WorldPos{.w = 0.0, .h = 0.0, .d = 100.0}, kPointCollider
  );

  // 自機を軸にすると、同じ角度なら近い方が選ばれる
  REQUIRE(
      LockOnSystem::SelectByDirection(
          registry, axis, Vec2{0.0, -1.0}, kLockConfig
      ) == near
  );
  // 軸を near へ移すと、その先の far が選ばれる（near 自身は返らない）
  REQUIRE(
      LockOnSystem::SelectByDirection(
          registry, near, Vec2{0.0, -1.0}, kLockConfig
      ) == far
  );
}

TEST_CASE(
    "LockOnSystem::SelectByDirection - excludes candidates beyond "
    "angle_limit_deg"
) {
  entt::registry registry;
  const auto axis = MakeEnemy(registry, WorldPos{}, kPointCollider);
  // dir（画面上方向）から90度ずれた位置（w方向のみ）は angleLimitDeg=60
  // を超える
  MakeEnemy(registry, WorldPos{.w = 50.0, .h = 0.0, .d = 0.0}, kPointCollider);
  REQUIRE(
      (LockOnSystem::SelectByDirection(
           registry, axis, Vec2{0.0, -1.0}, kLockConfig
       ) == entt::null)
  );
}

TEST_CASE("LockOnSystem::SelectByDirection - excludes Defeated enemies") {
  entt::registry registry;
  const auto axis = MakeEnemy(registry, WorldPos{}, kPointCollider);
  MakeEnemy(
      registry, WorldPos{.w = 0.0, .h = 0.0, .d = 50.0}, kPointCollider,
      EnemyMotion::Defeated{}
  );
  REQUIRE(
      (LockOnSystem::SelectByDirection(
           registry, axis, Vec2{0.0, -1.0}, kLockConfig
       ) == entt::null)
  );
}

TEST_CASE(
    "LockOnSystem::SelectByDirection - picks the aligned candidate within "
    "range"
) {
  entt::registry registry;
  const auto axis = MakeEnemy(registry, WorldPos{}, kPointCollider);
  // dir（画面上方向）の正面（d が正）
  const auto aligned = MakeEnemy(
      registry, WorldPos{.w = 0.0, .h = 0.0, .d = 50.0}, kPointCollider
  );
  REQUIRE(
      LockOnSystem::SelectByDirection(
          registry, axis, Vec2{0.0, -1.0}, kLockConfig
      ) == aligned
  );
}

TEST_CASE(
    "LockOnSystem::SelectByDirection - angle weighting can prefer a farther "
    "but better-aligned candidate over a nearer, more off-axis one"
) {
  entt::registry registry;
  const auto axis = MakeEnemy(registry, WorldPos{}, kPointCollider);
  // P: 軸から距離50・角度50度（dir=画面上方向から時計回りに50度傾けた位置）
  const auto p = MakeEnemy(
      registry, WorldPos{.w = 38.30, .h = 0.0, .d = 32.14}, kPointCollider
  );
  // Q: 軸から距離60・角度0度（正面）
  const auto q = MakeEnemy(
      registry, WorldPos{.w = 0.0, .h = 0.0, .d = 60.0}, kPointCollider
  );
  // score(P) = 50 * (1 + 50/60) ≈ 91.7、score(Q) = 60 * (1 + 0) = 60 なので
  // 単純な最近傍とは異なり、より正面に近い Q が選ばれる
  const auto selected = LockOnSystem::SelectByDirection(
      registry, axis, Vec2{0.0, -1.0}, kLockConfig
  );
  REQUIRE(selected == q);
  REQUIRE(selected != p);
}

// ---- Update ----

TEST_CASE(
    "LockOnSystem::Update - mouse rule locks onto the enemy under the "
    "cursor and spawns a target reticle"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry, WorldPos{});
  const auto enemy = MakeEnemy(
      registry, WorldPos{.w = 50.0, .h = 0.0, .d = 0.0},
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );

  const FrameData frameData{
      .input = InputState{.pointerPos = Scene::Center() + Vec2{50.0, 0.0}}
  };
  LockOnSystem::Update(registry, frameData);

  REQUIRE(registry.get<LockOn>(player).target == enemy);
  REQUIRE((registry.get<LockOn>(player).halfTarget == entt::null));
  const auto reticle = registry.get<LockOn>(player).targetReticle;
  REQUIRE(registry.valid(reticle));
  REQUIRE(registry.get<Hierarchy>(reticle).parent() == enemy);
}

TEST_CASE(
    "LockOnSystem::Update - mouse rule clears the target and destroys the "
    "reticle once the cursor moves off"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry, WorldPos{});
  MakeEnemy(
      registry, WorldPos{.w = 50.0, .h = 0.0, .d = 0.0},
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );

  LockOnSystem::Update(
      registry,
      FrameData{
          .input = InputState{.pointerPos = Scene::Center() + Vec2{50.0, 0.0}}
      }
  );
  const auto reticle = registry.get<LockOn>(player).targetReticle;
  REQUIRE(registry.valid(reticle));

  LockOnSystem::Update(
      registry,
      FrameData{
          .input =
              InputState{.pointerPos = Scene::Center() + Vec2{9999.0, 9999.0}}
      }
  );

  REQUIRE((registry.get<LockOn>(player).target == entt::null));
  REQUIRE_FALSE(registry.valid(reticle));
}

TEST_CASE(
    "LockOnSystem::Update - stick rule half-locks while tilted and confirms "
    "the target on release"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry, WorldPos{});
  const auto enemy = MakeEnemy(
      registry, WorldPos{.w = 0.0, .h = 0.0, .d = 50.0}, kPointCollider
  );

  // 傾き0.5以上でスティック規則に入り、正面の敵を半ロックする
  LockOnSystem::Update(
      registry, FrameData{.input = InputState{.lockAxis = Vec2{0.0, -1.0}}}
  );
  REQUIRE(registry.get<LockOn>(player).halfTarget == enemy);
  REQUIRE((registry.get<LockOn>(player).target == entt::null));
  const auto halfReticle = registry.get<LockOn>(player).halfReticle;
  REQUIRE(registry.valid(halfReticle));

  // 離し0.24以下で確定へ移す（SelectByDirection はやり直さない）
  LockOnSystem::Update(
      registry, FrameData{.input = InputState{.lockAxis = Vec2::Zero()}}
  );
  REQUIRE(registry.get<LockOn>(player).target == enemy);
  REQUIRE((registry.get<LockOn>(player).halfTarget == entt::null));
  REQUIRE_FALSE(registry.valid(halfReticle));
}

TEST_CASE(
    "LockOnSystem::Update - stick rule releasing with no candidate leaves "
    "the lock cleared"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry, WorldPos{});

  LockOnSystem::Update(
      registry, FrameData{.input = InputState{.lockAxis = Vec2{0.0, -1.0}}}
  );
  REQUIRE((registry.get<LockOn>(player).halfTarget == entt::null));

  LockOnSystem::Update(
      registry, FrameData{.input = InputState{.lockAxis = Vec2::Zero()}}
  );
  REQUIRE((registry.get<LockOn>(player).target == entt::null));
}

TEST_CASE("LockOnSystem::Update - clears the target once it becomes Defeated") {
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry, WorldPos{});
  const auto enemy = MakeEnemy(
      registry, WorldPos{.w = 50.0, .h = 0.0, .d = 0.0},
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );

  LockOnSystem::Update(
      registry,
      FrameData{
          .input = InputState{.pointerPos = Scene::Center() + Vec2{50.0, 0.0}}
      }
  );
  REQUIRE(registry.get<LockOn>(player).target == enemy);
  const auto reticle = registry.get<LockOn>(player).targetReticle;
  REQUIRE(registry.valid(reticle));

  registry.replace<EnemyMotion::Variant>(enemy, EnemyMotion::Defeated{});
  // カーソルを敵から離し、マウス規則による再ロックが起きないようにする
  LockOnSystem::Update(
      registry,
      FrameData{
          .input =
              InputState{.pointerPos = Scene::Center() + Vec2{9999.0, 9999.0}}
      }
  );

  REQUIRE((registry.get<LockOn>(player).target == entt::null));
  REQUIRE_FALSE(registry.valid(reticle));
}

#endif
