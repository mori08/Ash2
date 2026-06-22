#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/Motion.hpp"
#include "Component/Player.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/AnimationData.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/MotionSystem.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace {

/// @brief テスト用の registry.ctx() セットアップ（PlayerConfig +
/// AnimationDataRegistry）
void SetupContext(entt::registry& registry) {
  registry.ctx().emplace<PlayerConfig>(PlayerConfig{
      .speed = 100.0,
      .jumpSpeed = 300.0,
      .gravity = 800.0,
      .melee = {.capMidH = 40.0,
                .reach = 50.0,
                .radius = 20.0,
                .damage = 10,
                .windupSec = 0.05,
                .activeSec = 0.10,
                .recoverySec = 0.20,
                .active2Sec = 0.15,
                .slashRiseHeight = 40.0,
                .windup3Sec = 0.10,
                .active3Sec = 0.20,
                .recovery3Sec = 0.20,
                .radius3 = 25.0},
      .ranged = {.reach = 100.0,
                 .radius = 5.0,
                 .damage = 5,
                 .bulletSpeed = 300.0,
                 .spawnHeight = 40.0},
  });

  AnimationDataRegistry animReg;
  AnimationData playerData{
      .textureKey = U"player",
      .size = {32, 32},
      .drawOffset = {0, 0},
  };
  playerData.clips[U"idle"] = AnimationClip{.row = 0, .count = 4, .speed = 4.0};
  playerData.clips[U"move"] = AnimationClip{.row = 1, .count = 4, .speed = 8.0};
  playerData.clips[U"jump"] = AnimationClip{.row = 2, .count = 1, .speed = 1.0};
  playerData.clips[U"melee_1"] =
      AnimationClip{.row = 3, .count = 6, .speed = 12.0};
  playerData.clips[U"ranged_attack"] =
      AnimationClip{.row = 4, .count = 4, .speed = 8.0};
  animReg[U"player"] = playerData;
  registry.ctx().emplace<AnimationDataRegistry>(std::move(animReg));
}

/// @brief テスト用のプレイヤーエンティティを生成する（地上・Neutral 付き）
entt::entity MakePlayer(entt::registry& registry) {
  const auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Velocity>(player);
  registry.emplace<SpriteAnimation>(
      player, SpriteAnimation{.dataKey = U"player", .currentClip = U"idle"});
  registry.emplace<Motion>(player, PlayerMotion::Neutral{});
  return player;
}

}  // namespace

TEST_CASE(
    "PlayerMotionSystem - melee attack input transitions Neutral to Melee1") {
  // 近接攻撃入力で Neutral から Melee1 へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.attackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee1>(motion));

  const auto& melee = std::get<PlayerMotion::Melee1>(motion);
  REQUIRE(melee.elapsed == Approx(0.0));
  // 構え中（windupSec 未満）はヒットボックス未生成
  REQUIRE(melee.hitboxEntity == entt::entity{entt::null});
  REQUIRE_FALSE(melee.comboQueued);

  REQUIRE(registry.get<SpriteAnimation>(player).currentClip == U"melee_1");
}

TEST_CASE(
    "PlayerMotionSystem - ranged attack input transitions Neutral to "
    "Ranged") {
  // 遠距離攻撃入力で Neutral から Ranged へ遷移し、弾エンティティが生成される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.rangedAttackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Ranged>(motion));

  const auto& ranged = std::get<PlayerMotion::Ranged>(motion);
  REQUIRE(ranged.timer == Approx(4.0 / 8.0));

  REQUIRE(registry.get<SpriteAnimation>(player).currentClip ==
          U"ranged_attack");

  // 弾エンティティが生成されている
  const auto bulletView = registry.view<Projectile>();
  REQUIRE(bulletView.size() == 1);
}

TEST_CASE("PlayerMotionSystem - no attack input keeps Neutral") {
  // 攻撃入力がない場合は Neutral のまま
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const FrameData frameData{};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - airborne player ignores attack input") {
  // 空中にいる場合は攻撃入力を無視する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;

  FrameData frameData{};
  frameData.input.attackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - jump input immediately switches to jump clip") {
  // ジャンプ入力と同フレームで jump クリップに切り替わる（1フレーム遅延の修正）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.jumpDown = true;

  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(player).h == Approx(300.0));
  REQUIRE(registry.get<SpriteAnimation>(player).currentClip == U"jump");
}

TEST_CASE("PlayerMotionSystem - elapsed expiry transitions Melee1 to Neutral") {
  // elapsed が windupSec+activeSec+recoverySec を超え、comboQueued
  // が立っていない場合は Melee1 から Neutral
  // へ遷移し、ヒットボックスが残っていれば破棄する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // ヒットボックスエンティティ（Melee1.hitboxEntity 用のダミー）
  const auto hitbox = registry.create();
  registry.replace<Motion>(
      player, PlayerMotion::Melee1{.elapsed = 0.34, .hitboxEntity = hitbox});

  const FrameData frameData{.dt = 0.5};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
  REQUIRE_FALSE(registry.valid(hitbox));
}

TEST_CASE("PlayerMotionSystem - timer expiry transitions Ranged to Neutral") {
  // timer <= 0 になったら Ranged から Neutral へ遷移する（hitbox を持たない）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(player, PlayerMotion::Ranged{.timer = 0.1});

  const FrameData frameData{.dt = 0.5};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - Melee1 elapsed increases but stays in Melee1") {
  // recoveryEnd 未満の間は Melee1 のまま、elapsed が加算される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee1{.elapsed = 0.0, .hitboxEntity = entt::null});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee1>(motion));
  REQUIRE(std::get<PlayerMotion::Melee1>(motion).elapsed == Approx(0.1));
}

TEST_CASE(
    "PlayerMotionSystem - Melee1 spawns hitbox when entering active frame") {
  // 構え時間（windupSec）を超えた瞬間にヒットボックス（光の珠）が生成される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee1{.elapsed = 0.0, .hitboxEntity = entt::null});

  // windupSec(0.05) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee1>(motion);
  REQUIRE(melee.hitboxEntity != entt::entity{entt::null});
  REQUIRE(registry.valid(melee.hitboxEntity));
  REQUIRE(registry.all_of<Collider, Attack, LocalOffset>(melee.hitboxEntity));

  const auto& attack = registry.get<Attack>(melee.hitboxEntity);
  REQUIRE(attack.hitstopSec > 0.0);
}

TEST_CASE(
    "PlayerMotionSystem - Melee1 destroys hitbox when entering recovery") {
  // 攻撃判定終了（windupSec+activeSec）を過ぎたらヒットボックスが破棄される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player, PlayerMotion::Melee1{.elapsed = 0.10, .hitboxEntity = hitbox});

  // windupSec+activeSec(0.15) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee1>(motion);
  REQUIRE(melee.hitboxEntity == entt::entity{entt::null});
  REQUIRE_FALSE(registry.valid(hitbox));
}

TEST_CASE("PlayerMotionSystem - Melee1 stops horizontal movement") {
  // Melee1 中は移動入力があっても横移動が止まる
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee1{.elapsed = 0.0, .hitboxEntity = entt::null});

  FrameData frameData{.dt = 0.1};
  frameData.input.moveAxis = Vec2{1.0, 0.0};
  MotionSystem::Update(registry, frameData);

  const auto& vel = registry.get<Velocity>(player);
  REQUIRE(vel.w == Approx(0.0));
  REQUIRE(vel.d == Approx(0.0));
}

TEST_CASE(
    "PlayerMotionSystem - Melee1 attack input during windup only sets "
    "comboQueued") {
  // windup中（elapsed < activeStart）の攻撃入力では即時遷移せず、
  // comboQueued が立つのみ
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee1{.elapsed = 0.0, .hitboxEntity = entt::null});

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee1>(motion));
  REQUIRE(std::get<PlayerMotion::Melee1>(motion).comboQueued);
}

TEST_CASE(
    "PlayerMotionSystem - Melee1 attack input during active only sets "
    "comboQueued") {
  // active中（activeStart <= elapsed < activeEnd）の攻撃入力でも
  // 即時遷移せず、comboQueued が立つのみ
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // windupSec(0.05) を超え activeEnd(0.15) 未満
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee1{.elapsed = 0.10, .hitboxEntity = entt::null});

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee1>(motion));
  REQUIRE(std::get<PlayerMotion::Melee1>(motion).comboQueued);
}

TEST_CASE(
    "PlayerMotionSystem - Melee1 transitions to Melee2 immediately when "
    "comboQueued at activeEnd") {
  // activeEnd 到達時に comboQueued が立っていれば即座に Melee2 へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd は windupSec+activeSec = 0.15
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee1{
          .elapsed = 0.14, .hitboxEntity = entt::null, .comboQueued = true});

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee2>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Melee1 transitions to Melee2 immediately on new "
    "attack input during recovery") {
  // recovery中（elapsed >= activeEnd）の新規攻撃入力で即座に Melee2 へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd(0.15) を既に過ぎている状態（comboQueued は未予約）
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee1{.elapsed = 0.16, .hitboxEntity = entt::null});

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee2>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Melee1 transitions to Neutral when recoveryEnd "
    "exceeded without comboQueued") {
  // comboQueued が立っていない状態で recoveryEnd を超えたら Neutral へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // recoveryEnd は windupSec+activeSec+recoverySec = 0.35
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee1{.elapsed = 0.34, .hitboxEntity = entt::null});

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - Melee1 comboQueued accumulates across frames") {
  // comboQueued は一度立ったら立ったまま（OR的に蓄積）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee1{.elapsed = 0.0, .hitboxEntity = entt::null});

  // 1フレーム目：攻撃入力あり、comboQueued が立つ
  FrameData frameData1{.dt = 0.01};
  frameData1.input.attackDown = true;
  MotionSystem::Update(registry, frameData1);

  REQUIRE(
      std::get<PlayerMotion::Melee1>(registry.get<Motion>(player)).comboQueued);

  // 2フレーム目：攻撃入力なし、comboQueued は立ったまま
  const FrameData frameData2{.dt = 0.01};
  MotionSystem::Update(registry, frameData2);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee1>(motion));
  REQUIRE(std::get<PlayerMotion::Melee1>(motion).comboQueued);
}

TEST_CASE("PlayerMotionSystem - Melee2 elapsed increases but stays in Melee2") {
  // recoveryEnd 未満の間は Melee2 のまま、elapsed が加算される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee2{.elapsed = 0.0, .hitboxEntity = entt::null});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee2>(motion));
  REQUIRE(std::get<PlayerMotion::Melee2>(motion).elapsed == Approx(0.1));
}

TEST_CASE(
    "PlayerMotionSystem - Melee2 spawns hitbox when entering active frame") {
  // 構え時間（windupSec）を超えた瞬間にヒットボックス（光の珠）が生成される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee2{.elapsed = 0.0, .hitboxEntity = entt::null});

  // windupSec(0.05) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee2>(motion);
  REQUIRE(melee.hitboxEntity != entt::entity{entt::null});
  REQUIRE(registry.valid(melee.hitboxEntity));
  REQUIRE(registry.all_of<Collider, Attack, LocalOffset>(melee.hitboxEntity));
}

TEST_CASE(
    "PlayerMotionSystem - Melee2 destroys hitbox when entering recovery") {
  // 攻撃判定終了（windupSec+active2Sec）を過ぎたらヒットボックスが破棄される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player, PlayerMotion::Melee2{.elapsed = 0.15, .hitboxEntity = hitbox});

  // windupSec+active2Sec(0.20) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee2>(motion);
  REQUIRE(melee.hitboxEntity == entt::entity{entt::null});
  REQUIRE_FALSE(registry.valid(hitbox));
}

TEST_CASE(
    "PlayerMotionSystem - Melee2 transitions to Neutral when recoveryEnd "
    "exceeded") {
  // recoveryEnd を超えたら Neutral へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // recoveryEnd は windupSec+active2Sec+recoverySec = 0.40
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee2{.elapsed = 0.39, .hitboxEntity = entt::null});

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Melee2 attack input during windup only sets "
    "comboQueued") {
  // windup中（elapsed < activeStart）の攻撃入力では即時遷移せず、
  // comboQueued が立つのみ
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee2{.elapsed = 0.0, .hitboxEntity = entt::null});

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee2>(motion));
  REQUIRE(std::get<PlayerMotion::Melee2>(motion).comboQueued);
}

TEST_CASE(
    "PlayerMotionSystem - Melee2 transitions to Melee3 immediately when "
    "comboQueued at activeEnd") {
  // activeEnd 到達時に comboQueued が立っていれば即座に Melee3 へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd は windupSec+active2Sec = 0.20
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee2{
          .elapsed = 0.19, .hitboxEntity = entt::null, .comboQueued = true});

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee3>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Melee2 transitions to Melee3 immediately on new "
    "attack input during recovery") {
  // recovery中（elapsed >= activeEnd）の新規攻撃入力で即座に Melee3 へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd(0.20) を既に過ぎている状態（comboQueued は未予約）
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee2{.elapsed = 0.21, .hitboxEntity = entt::null});

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee3>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Melee2 transitions to Neutral when recoveryEnd "
    "exceeded without comboQueued") {
  // comboQueued が立っていない状態で recoveryEnd を超えたら Neutral へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // recoveryEnd は windupSec+active2Sec+recoverySec = 0.40
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee2{.elapsed = 0.39, .hitboxEntity = entt::null});

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - Melee3 elapsed increases but stays in Melee3") {
  // recoveryEnd 未満の間は Melee3 のまま、elapsed が加算される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee3{.elapsed = 0.0, .hitboxEntity = entt::null});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee3>(motion));
  REQUIRE(std::get<PlayerMotion::Melee3>(motion).elapsed == Approx(0.1));
}

TEST_CASE(
    "PlayerMotionSystem - Melee3 spawns hitbox when entering active frame") {
  // 構え時間（windup3Sec）を超えた瞬間にヒットボックス（光の珠）が生成される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee3{.elapsed = 0.0, .hitboxEntity = entt::null});

  // windup3Sec(0.10) を跨ぐ dt
  const FrameData frameData{.dt = 0.11};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee3>(motion);
  REQUIRE(melee.hitboxEntity != entt::entity{entt::null});
  REQUIRE(registry.valid(melee.hitboxEntity));
  REQUIRE(registry.all_of<Collider, Attack, LocalOffset>(melee.hitboxEntity));

  const auto& attack = registry.get<Attack>(melee.hitboxEntity);
  REQUIRE(attack.hitstopSec > 0.0);
}

TEST_CASE(
    "PlayerMotionSystem - Melee3 destroys hitbox when entering recovery") {
  // 攻撃判定終了（windup3Sec+active3Sec）を過ぎたらヒットボックスが破棄される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player, PlayerMotion::Melee3{.elapsed = 0.29, .hitboxEntity = hitbox});

  // windup3Sec+active3Sec(0.30) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee3>(motion);
  REQUIRE(melee.hitboxEntity == entt::entity{entt::null});
  REQUIRE_FALSE(registry.valid(hitbox));
}

TEST_CASE(
    "PlayerMotionSystem - Melee3 transitions to Neutral when recoveryEnd "
    "exceeded") {
  // recoveryEnd を超えたら Neutral へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // recoveryEnd は windup3Sec+active3Sec+recovery3Sec = 0.50
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee3{.elapsed = 0.49, .hitboxEntity = entt::null});

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - Melee3 ignores attack input (finisher)") {
  // Melee3 は締め技のため攻撃入力を無視し、recoveryEnd 未満では Melee3 のまま
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee3{.elapsed = 0.0, .hitboxEntity = entt::null});

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee3>(motion));
}

TEST_CASE("PlayerMotionSystem - Melee3 stops horizontal movement") {
  // Melee3 中は移動入力があっても横移動が止まる
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee3{.elapsed = 0.0, .hitboxEntity = entt::null});

  FrameData frameData{.dt = 0.1};
  frameData.input.moveAxis = Vec2{1.0, 0.0};
  MotionSystem::Update(registry, frameData);

  const auto& vel = registry.get<Velocity>(player);
  REQUIRE(vel.w == Approx(0.0));
  REQUIRE(vel.d == Approx(0.0));
}

#endif
