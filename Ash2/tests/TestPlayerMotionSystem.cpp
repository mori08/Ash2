#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/FadeOut.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hitstop.hpp"
#include "Component/Invincible.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/Motion.hpp"
#include "Component/Player.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Stamina.hpp"
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
      .melee =
          {
              .capMidH = 40.0,
              .reach = 50.0,
              .damage = 10,
              .stages =
                  {
                      // 1段目相当（突き出し）
                      MeleeStageConfig{
                          .timeline =
                              {.windupSec = 0.05,
                               .activeSec = 0.10,
                               .recoveryASec = 0.0,
                               .recoveryBSec = 0.20},
                          .radius = 20.0,
                          .trajectory = MeleeTrajectory::Thrust,
                          .hitstopSec = 0.05,
                      },
                      // 2段目相当（斬り上げ）
                      MeleeStageConfig{
                          .timeline =
                              {.windupSec = 0.05,
                               .activeSec = 0.15,
                               .recoveryASec = 0.0,
                               .recoveryBSec = 0.20},
                          .radius = 20.0,
                          .trajectory = MeleeTrajectory::Slash,
                          .slashRiseHeight = 40.0,
                          .hitstopSec = 0.05,
                      },
                      // 3段目相当（締め技、突き出し）
                      MeleeStageConfig{
                          .timeline =
                              {.windupSec = 0.10,
                               .activeSec = 0.20,
                               .recoveryASec = 0.0,
                               .recoveryBSec = 0.20},
                          .radius = 25.0,
                          .trajectory = MeleeTrajectory::Thrust,
                          .hitstopSec = 0.13,
                      },
                  },
          },
      .ranged =
          {.reach = 100.0,
           .radius = 5.0,
           .damage = 5,
           .bulletSpeed = 300.0,
           .spawnHeight = 40.0},
      .dash =
          {.speed = 500.0,
           .timeline =
               {.windupSec = 0.0,
                .activeSec = 0.15,
                .recoveryASec = 0.15,
                .recoveryBSec = 0.15},
           .staminaCost = 20},
      .dashAttack =
          {.timeline =
               {.windupSec = 0.05,
                .activeSec = 0.20,
                .recoveryASec = 0.20,
                .recoveryBSec = 0.0},
           .speed = 600.0,
           .orbitRadius = 30.0,
           .radius = 20.0,
           .damage = 20,
           .hitstopSec = 0.08},
      .airAttack =
          {.timeline =
               {.windupSec = 0.05,
                .activeSec = 0.25,
                .recoveryASec = 0.20,
                .recoveryBSec = 0.0},
           .orbitRadius = 50.0,
           .radius = 20.0,
           .damage = 15,
           .hitstopSec = 0.08},
      .landing = {.recoverySec = 0.20},
      .attackEffect = {.fadeSec = 0.30},
  });

  AnimationDataRegistry animReg;
  AnimationData playerData{
      .textureKey = U"player",
      .size = {32, 32},
      .drawOffset = {0, 0},
  };
  playerData.clips[U"idle"] = AnimationClip{.row = 0, .count = 4, .speed = 4.0};
  playerData.clips[U"move"] = AnimationClip{.row = 1, .count = 4, .speed = 8.0};
  playerData.clips[U"jump_rise"] =
      AnimationClip{.row = 2, .count = 1, .speed = 1.0};
  playerData.clips[U"jump_fall"] =
      AnimationClip{.row = 3, .count = 1, .speed = 1.0};
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
      player, SpriteAnimation{.dataKey = U"player", .currentClip = U"idle"}
  );
  registry.emplace<Motion>(player, PlayerMotion::Neutral{});
  registry.emplace<Stamina>(player, Stamina{.max = 100, .current = 100});
  return player;
}

}  // namespace

TEST_CASE(
    "PlayerMotionSystem - melee attack input transitions Neutral to Melee "
    "stage 0"
) {
  // 近接攻撃入力で Neutral から Melee（1段目）へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.attackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));

  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  REQUIRE(melee.stage == 0);
  REQUIRE(melee.elapsed == Approx(0.0));
  // 構え中（windupSec 未満）はヒットボックス未生成
  REQUIRE(melee.hitboxEntity == entt::entity{entt::null});
  REQUIRE_FALSE(melee.comboQueued);

  REQUIRE(registry.get<SpriteAnimation>(player).currentClip == U"melee_1");
}

TEST_CASE(
    "PlayerMotionSystem - ranged attack input transitions Neutral to "
    "Ranged"
) {
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

  REQUIRE(
      registry.get<SpriteAnimation>(player).currentClip == U"ranged_attack"
  );

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

TEST_CASE(
    "PlayerMotionSystem - airborne player attack input transitions Neutral "
    "to AirAttack"
) {
  // 空中にいる場合は攻撃入力で AirAttack へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;

  FrameData frameData{};
  frameData.input.attackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::AirAttack>(motion));

  const auto& air = std::get<PlayerMotion::AirAttack>(motion);
  REQUIRE(air.elapsed == Approx(0.0));
  // 構え中（windupSec 未満）はヒットボックス未生成
  REQUIRE(air.hitboxEntity == entt::entity{entt::null});

  REQUIRE(registry.get<SpriteAnimation>(player).currentClip == U"air_attack");
}

TEST_CASE(
    "PlayerMotionSystem - airborne player ranged attack input transitions "
    "Neutral to Ranged"
) {
  // 空中にいる場合も遠距離攻撃入力で Ranged
  // へ遷移し、弾エンティティが生成される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 100.0;

  FrameData frameData{};
  frameData.input.rangedAttackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Ranged>(motion));

  const auto& ranged = std::get<PlayerMotion::Ranged>(motion);
  REQUIRE(ranged.timer == Approx(4.0 / 8.0));

  REQUIRE(
      registry.get<SpriteAnimation>(player).currentClip == U"ranged_attack"
  );

  // 弾エンティティが生成されている
  const auto bulletView = registry.view<Projectile>();
  REQUIRE(bulletView.size() == 1);
}

TEST_CASE(
    "PlayerMotionSystem - AirAttack spawns hitbox and moves along vertical "
    "orbit during active frame"
) {
  // 攻撃判定区間でヒットボックスが生成され、w-h 平面上の円軌道で移動する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 100.0;  // 空中（接地遷移を避ける）

  registry.replace<Motion>(
      player,
      PlayerMotion::AirAttack{.elapsed = 0.0, .hitboxEntity = entt::null}
  );

  // windupSec(0.05) を跨ぎ、activeSec(0.25) 中の進行度 0.25 地点まで進める
  const FrameData frameData{.dt = 0.1125};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& air = std::get<PlayerMotion::AirAttack>(motion);
  REQUIRE(air.hitboxEntity != entt::entity{entt::null});
  REQUIRE(registry.valid(air.hitboxEntity));
  REQUIRE(registry.all_of<Collider, Attack, LocalOffset>(air.hitboxEntity));

  // 進行度0.25 → 円の頂点（w=0、h=capMidH(40.0)-orbitRadius(50.0)=-10.0）
  const auto& localOffset = registry.get<LocalOffset>(air.hitboxEntity);
  REQUIRE(localOffset.value.w == Approx(0.0).margin(0.001));
  REQUIRE(localOffset.value.h == Approx(-10.0));
  REQUIRE(localOffset.value.d == Approx(0.0));

  const auto& attack = registry.get<Attack>(air.hitboxEntity);
  REQUIRE(attack.damage == 15);
}

TEST_CASE(
    "PlayerMotionSystem - AirAttack orbit mirrors horizontally when facing "
    "left"
) {
  // 左向きのとき w 成分の符号が反転する（progress 0.25 は cos=0 で符号の
  // 影響を受けないため、progress 0.0 で検証する）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 100.0;  // 空中（接地遷移を避ける）
  registry.get<SpriteAnimation>(player).facingRight = false;

  registry.replace<Motion>(
      player,
      PlayerMotion::AirAttack{.elapsed = 0.0, .hitboxEntity = entt::null}
  );

  // windupSec(0.05) ちょうどまで進め、progress 0.0（w = -orbitRadius）にする
  const FrameData frameData{.dt = 0.05};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& air = std::get<PlayerMotion::AirAttack>(motion);
  REQUIRE(air.hitboxEntity != entt::entity{entt::null});

  // 進行度0.0 → w = -orbitRadius(50.0)、h = capMidH(40.0)
  const auto& localOffset = registry.get<LocalOffset>(air.hitboxEntity);
  REQUIRE(localOffset.value.w == Approx(-50.0));
  REQUIRE(localOffset.value.h == Approx(40.0));
  REQUIRE(localOffset.value.d == Approx(0.0));
}

TEST_CASE(
    "PlayerMotionSystem - AirAttack transitions to Landing when player "
    "lands, destroying leftover hitbox"
) {
  // 後隙中でも接地したら Landing へ強制遷移し、ヒットボックスは破棄される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 0.0;  // 接地

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player, PlayerMotion::AirAttack{.elapsed = 0.1, .hitboxEntity = hitbox}
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Landing>(motion));
  // ヒットボックスは即座に破棄せず、Attack を外して FadeOut
  // へ引き渡す（親からも切り離される）
  REQUIRE(registry.valid(hitbox));
  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));
  REQUIRE(registry.all_of<FadeOut>(hitbox));
  REQUIRE_FALSE(registry.all_of<Hierarchy>(hitbox));

  const auto& landing = std::get<PlayerMotion::Landing>(motion);
  REQUIRE(landing.timer == Approx(0.20));
}

TEST_CASE(
    "PlayerMotionSystem - AirAttack transitions to Neutral on recovery "
    "timeout while still airborne"
) {
  // 接地せずに後隙が満了した場合はタイマー満了で Neutral へ戻る
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 500.0;  // 十分な高さで着地しない

  // recoveryBEnd は windupSec+activeSec+recoveryASec+recoveryBSec(0)
  // = 0.05+0.25+0.20 = 0.50
  registry.replace<Motion>(
      player,
      PlayerMotion::AirAttack{.elapsed = 0.49, .hitboxEntity = entt::null}
  );

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - jump input immediately switches to jump_rise "
    "clip"
) {
  // ジャンプ入力と同フレームで jump_rise
  // クリップに切り替わる（1フレーム遅延の修正）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.jumpDown = true;

  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(player).h == Approx(300.0));
  REQUIRE(registry.get<SpriteAnimation>(player).currentClip == U"jump_rise");
}

TEST_CASE(
    "PlayerMotionSystem - simultaneous jump and melee attack input "
    "transitions to Melee without vertical velocity"
) {
  // 同一フレームのジャンプ＋近接攻撃入力は Melee へ遷移し、上昇しない
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.jumpDown = true;
  frameData.input.attackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(registry.get<Velocity>(player).h == Approx(0.0));
}

TEST_CASE(
    "PlayerMotionSystem - simultaneous jump and ranged attack input "
    "transitions to Ranged without vertical velocity"
) {
  // 同一フレームのジャンプ＋遠距離攻撃入力は Ranged へ遷移し、上昇しない
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.jumpDown = true;
  frameData.input.rangedAttackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Ranged>(motion));
  REQUIRE(registry.get<Velocity>(player).h == Approx(0.0));
}

TEST_CASE(
    "PlayerMotionSystem - simultaneous jump and dash input transitions to "
    "Dash without vertical velocity"
) {
  // 同一フレームのジャンプ＋ダッシュ入力は Dash へ遷移し、上昇しない
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.jumpDown = true;
  frameData.input.dashDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  REQUIRE(registry.get<Velocity>(player).h == Approx(0.0));
}

TEST_CASE("PlayerMotionSystem - elapsed expiry transitions Melee to Neutral") {
  // elapsed が windupSec+activeSec+recoveryBSec を超え、comboQueued
  // が立っていない場合は Melee から Neutral
  // へ遷移し、ヒットボックスが残っていれば解放する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // ヒットボックスエンティティ（Melee.hitboxEntity 用のダミー）
  const auto hitbox = registry.create();
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{.stage = 0, .elapsed = 0.34, .hitboxEntity = hitbox}
  );

  const FrameData frameData{.dt = 0.5};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
  // ヒットボックスは即座に破棄せず、Attack を外して FadeOut
  // へ引き渡す（親からも切り離される）
  REQUIRE(registry.valid(hitbox));
  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));
  REQUIRE(registry.all_of<FadeOut>(hitbox));
  REQUIRE_FALSE(registry.all_of<Hierarchy>(hitbox));
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

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 elapsed increases but stays in "
    "Melee"
) {
  // recoveryEnd 未満の間は Melee のまま、elapsed が加算される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).elapsed == Approx(0.1));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 spawns hitbox when entering active "
    "frame"
) {
  // 構え時間（windupSec）を超えた瞬間にヒットボックス（光の珠）が生成される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  // windupSec(0.05) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  REQUIRE(melee.hitboxEntity != entt::entity{entt::null});
  REQUIRE(registry.valid(melee.hitboxEntity));
  REQUIRE(registry.all_of<Collider, Attack, LocalOffset>(melee.hitboxEntity));

  const auto& attack = registry.get<Attack>(melee.hitboxEntity);
  REQUIRE(attack.hitstopSec > 0.0);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 destroys hitbox when entering "
    "recovery"
) {
  // 攻撃判定終了（windupSec+activeSec）を過ぎたらヒットボックスが解放される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{.stage = 0, .elapsed = 0.10, .hitboxEntity = hitbox}
  );

  // windupSec+activeSec(0.15) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  REQUIRE(melee.hitboxEntity == entt::entity{entt::null});
  // ヒットボックスは即座に破棄せず、Attack を外して FadeOut
  // へ引き渡す（親からも切り離される）
  REQUIRE(registry.valid(hitbox));
  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));
  REQUIRE(registry.all_of<FadeOut>(hitbox));
  REQUIRE_FALSE(registry.all_of<Hierarchy>(hitbox));
}

TEST_CASE("PlayerMotionSystem - Melee stage 0 stops horizontal movement") {
  // Melee 中は移動入力があっても横移動が止まる
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.1};
  frameData.input.moveAxis = Vec2{1.0, 0.0};
  MotionSystem::Update(registry, frameData);

  const auto& vel = registry.get<Velocity>(player);
  REQUIRE(vel.w == Approx(0.0));
  REQUIRE(vel.d == Approx(0.0));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 attack input during windup only "
    "sets comboQueued"
) {
  // windup中（elapsed < activeStart）の攻撃入力では即時遷移せず、
  // comboQueued が立つのみ
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).comboQueued);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 attack input during active only "
    "sets comboQueued"
) {
  // active中（activeStart <= elapsed < activeEnd）の攻撃入力でも
  // 即時遷移せず、comboQueued が立つのみ
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // windupSec(0.05) を超え activeEnd(0.15) 未満
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.10, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).comboQueued);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 transitions to stage 1 immediately "
    "when comboQueued at activeEnd"
) {
  // activeEnd 到達時に comboQueued が立っていれば即座に次段へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd は windupSec+activeSec = 0.15
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0,
          .elapsed = 0.14,
          .hitboxEntity = entt::null,
          .comboQueued = true
      }
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).stage == 1);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 transitions to stage 1 immediately "
    "on new attack input during recovery"
) {
  // recovery中（elapsed >= activeEnd）の新規攻撃入力で即座に次段へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd(0.15) を既に過ぎている状態（comboQueued は未予約）
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.16, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).stage == 1);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 attack input during recoveryA only "
    "sets comboQueued and fires after entering recoveryB"
) {
  // 後隙A中（activeEnd <= elapsed < recoveryAEnd）の攻撃入力では即時遷移せず
  // 予約のみ行い、後隙Bに入ってから次段へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // 1段目に後隙Aを設定する（activeEnd=0.15、recoveryAEnd=0.25）
  registry.ctx().get<PlayerConfig>().melee.stages[0].timeline.recoveryASec =
      0.10;

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.16, .hitboxEntity = entt::null
      }
  );

  // 後隙A中の攻撃入力：予約のみで遷移しない
  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  {
    const auto& motion = registry.get<Motion>(player);
    REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
    REQUIRE(std::get<PlayerMotion::Melee>(motion).stage == 0);
    REQUIRE(std::get<PlayerMotion::Melee>(motion).comboQueued);
  }

  // 後隙Bへ入るまで進める（elapsed 0.17 → 0.27）：予約が発動して次段へ
  const FrameData advance{.dt = 0.10};
  MotionSystem::Update(registry, advance);

  {
    const auto& motion = registry.get<Motion>(player);
    REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
    REQUIRE(std::get<PlayerMotion::Melee>(motion).stage == 1);
  }
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 transitions to Neutral when "
    "recoveryEnd exceeded without comboQueued"
) {
  // comboQueued が立っていない状態で recoveryEnd を超えたら Neutral へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // recoveryEnd は windupSec+activeSec+recoveryBSec = 0.35
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.34, .hitboxEntity = entt::null
      }
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 comboQueued accumulates across "
    "frames"
) {
  // comboQueued は一度立ったら立ったまま（OR的に蓄積）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  // 1フレーム目：攻撃入力あり、comboQueued が立つ
  FrameData frameData1{.dt = 0.01};
  frameData1.input.attackDown = true;
  MotionSystem::Update(registry, frameData1);

  REQUIRE(
      std::get<PlayerMotion::Melee>(registry.get<Motion>(player)).comboQueued
  );

  // 2フレーム目：攻撃入力なし、comboQueued は立ったまま
  const FrameData frameData2{.dt = 0.01};
  MotionSystem::Update(registry, frameData2);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).comboQueued);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 1 elapsed increases but stays in "
    "Melee"
) {
  // recoveryEnd 未満の間は Melee のまま、elapsed が加算される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 1, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).elapsed == Approx(0.1));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 1 spawns hitbox when entering active "
    "frame"
) {
  // 構え時間（windupSec）を超えた瞬間にヒットボックス（光の珠）が生成される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 1, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  // windupSec(0.05) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  REQUIRE(melee.hitboxEntity != entt::entity{entt::null});
  REQUIRE(registry.valid(melee.hitboxEntity));
  REQUIRE(registry.all_of<Collider, Attack, LocalOffset>(melee.hitboxEntity));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 1 destroys hitbox when entering "
    "recovery"
) {
  // 攻撃判定終了（windupSec+activeSec）を過ぎたらヒットボックスが解放される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{.stage = 1, .elapsed = 0.15, .hitboxEntity = hitbox}
  );

  // windupSec+activeSec(0.20) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  REQUIRE(melee.hitboxEntity == entt::entity{entt::null});
  // ヒットボックスは即座に破棄せず、Attack を外して FadeOut
  // へ引き渡す（親からも切り離される）
  REQUIRE(registry.valid(hitbox));
  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));
  REQUIRE(registry.all_of<FadeOut>(hitbox));
  REQUIRE_FALSE(registry.all_of<Hierarchy>(hitbox));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 1 transitions to Neutral when "
    "recoveryEnd exceeded"
) {
  // recoveryEnd を超えたら Neutral へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // recoveryEnd は windupSec+activeSec+recoveryBSec = 0.40
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 1, .elapsed = 0.39, .hitboxEntity = entt::null
      }
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 1 attack input during windup only "
    "sets comboQueued"
) {
  // windup中（elapsed < activeStart）の攻撃入力では即時遷移せず、
  // comboQueued が立つのみ
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 1, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).comboQueued);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 1 transitions to stage 2 immediately "
    "when comboQueued at activeEnd"
) {
  // activeEnd 到達時に comboQueued が立っていれば即座に次段へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd は windupSec+activeSec = 0.20
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 1,
          .elapsed = 0.19,
          .hitboxEntity = entt::null,
          .comboQueued = true
      }
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).stage == 2);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 1 transitions to stage 2 immediately "
    "on new attack input during recovery"
) {
  // recovery中（elapsed >= activeEnd）の新規攻撃入力で即座に次段へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd(0.20) を既に過ぎている状態（comboQueued は未予約）
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 1, .elapsed = 0.21, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).stage == 2);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 1 transitions to Neutral when "
    "recoveryEnd exceeded without comboQueued"
) {
  // comboQueued が立っていない状態で recoveryEnd を超えたら Neutral へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // recoveryEnd は windupSec+activeSec+recoveryBSec = 0.40
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 1, .elapsed = 0.39, .hitboxEntity = entt::null
      }
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 2 elapsed increases but stays in "
    "Melee"
) {
  // recoveryEnd 未満の間は Melee のまま、elapsed が加算される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 2, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).elapsed == Approx(0.1));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 2 spawns hitbox when entering active "
    "frame"
) {
  // 構え時間（windupSec）を超えた瞬間にヒットボックス（光の珠）が生成される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 2, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  // windupSec(0.10) を跨ぐ dt
  const FrameData frameData{.dt = 0.11};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  REQUIRE(melee.hitboxEntity != entt::entity{entt::null});
  REQUIRE(registry.valid(melee.hitboxEntity));
  REQUIRE(registry.all_of<Collider, Attack, LocalOffset>(melee.hitboxEntity));

  const auto& attack = registry.get<Attack>(melee.hitboxEntity);
  REQUIRE(attack.hitstopSec > 0.0);
}

TEST_CASE(
    "PlayerMotionSystem - Melee hitstopSec differs per stage, following "
    "config"
) {
  // 締め技（3段目）は1段目より長いヒットストップ時間を持つ（コンボ段との連動）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );
  // windupSec(0.05) を跨ぐ dt
  MotionSystem::Update(registry, FrameData{.dt = 0.06});
  const auto stage0Hitbox =
      std::get<PlayerMotion::Melee>(registry.get<Motion>(player)).hitboxEntity;
  const double stage0HitstopSec = registry.get<Attack>(stage0Hitbox).hitstopSec;

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 2, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );
  // windupSec(0.10) を跨ぐ dt
  MotionSystem::Update(registry, FrameData{.dt = 0.11});
  const auto stage2Hitbox =
      std::get<PlayerMotion::Melee>(registry.get<Motion>(player)).hitboxEntity;
  const double stage2HitstopSec = registry.get<Attack>(stage2Hitbox).hitstopSec;

  REQUIRE(stage0HitstopSec == Approx(0.05));
  REQUIRE(stage2HitstopSec == Approx(0.13));
  REQUIRE(stage2HitstopSec > stage0HitstopSec);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 2 destroys hitbox when entering "
    "recovery"
) {
  // 攻撃判定終了（windupSec+activeSec）を過ぎたらヒットボックスが解放される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{.stage = 2, .elapsed = 0.29, .hitboxEntity = hitbox}
  );

  // windupSec+activeSec(0.30) を跨ぐ dt
  const FrameData frameData{.dt = 0.06};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  REQUIRE(melee.hitboxEntity == entt::entity{entt::null});
  // ヒットボックスは即座に破棄せず、Attack を外して FadeOut
  // へ引き渡す（親からも切り離される）
  REQUIRE(registry.valid(hitbox));
  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));
  REQUIRE(registry.all_of<FadeOut>(hitbox));
  REQUIRE_FALSE(registry.all_of<Hierarchy>(hitbox));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 2 transitions to Neutral when "
    "recoveryEnd exceeded"
) {
  // recoveryEnd を超えたら Neutral へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // recoveryEnd は windupSec+activeSec+recoveryBSec = 0.50
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 2, .elapsed = 0.49, .hitboxEntity = entt::null
      }
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - Melee stage 2 (finisher) ignores attack input"
) {
  // 最終段（次段を持たない）は締め技のため攻撃入力を無視し、
  // recoveryEnd 未満では Melee のまま
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 2, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.01};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).stage == 2);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 2 (finisher) ignores dash input "
    "during recovery"
) {
  // 最終段はキャンセル不可のため、後隙中のダッシュ入力を無視する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd(0.30) を既に過ぎている状態
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 2, .elapsed = 0.31, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.01};
  frameData.input.dashDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(registry.get<Stamina>(player).current == 100);
}

TEST_CASE("PlayerMotionSystem - Melee stage 2 stops horizontal movement") {
  // Melee 中は移動入力があっても横移動が止まる
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 2, .elapsed = 0.0, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.1};
  frameData.input.moveAxis = Vec2{1.0, 0.0};
  MotionSystem::Update(registry, frameData);

  const auto& vel = registry.get<Velocity>(player);
  REQUIRE(vel.w == Approx(0.0));
  REQUIRE(vel.d == Approx(0.0));
}

TEST_CASE("PlayerMotionSystem - dash input transitions Neutral to Dash") {
  // ダッシュ入力で Neutral から Dash へ遷移し、ST が1回分消費される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.dashDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  const auto& dash = std::get<PlayerMotion::Dash>(motion);
  REQUIRE(dash.elapsed == Approx(0.0));
  REQUIRE_FALSE(dash.air);

  // dash.staminaCost(20) 分が消費される
  REQUIRE(registry.get<Stamina>(player).current == 80);
}

TEST_CASE(
    "PlayerMotionSystem - dash input is ignored when stamina is "
    "insufficient"
) {
  // ST不足の場合はダッシュ入力を無視し Neutral のまま、ST も減らない
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<Stamina>(player).current = 10;

  FrameData frameData{};
  frameData.input.dashDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
  REQUIRE(registry.get<Stamina>(player).current == 10);
}

TEST_CASE("PlayerMotionSystem - Dash grants Invincible during windup and dash"
) {
  // 構え・ダッシュ中（elapsed < activeEnd）は Invincible が付与される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // dash.timeline.windupSec(0.0) + activeSec(0.15) = 0.15 未満
  registry.replace<Motion>(player, PlayerMotion::Dash{.elapsed = 0.0});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.all_of<Invincible>(player));
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(registry.get<Motion>(player
  )));
}

TEST_CASE("PlayerMotionSystem - Dash removes Invincible when entering recovery"
) {
  // activeEnd(0.15) を超えた時点で Invincible が除去される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.emplace<Invincible>(player);
  registry.replace<Motion>(player, PlayerMotion::Dash{.elapsed = 0.14});

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  REQUIRE_FALSE(registry.all_of<Invincible>(player));
}

TEST_CASE(
    "PlayerMotionSystem - Dash moves in facing direction when no move "
    "input"
) {
  // ダッシュ中に移動入力がない場合は facingRight 方向へ移動する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<SpriteAnimation>(player).facingRight = true;

  registry.replace<Motion>(player, PlayerMotion::Dash{.elapsed = 0.0});

  const FrameData frameData{.dt = 0.05};
  MotionSystem::Update(registry, frameData);

  const auto& vel = registry.get<Velocity>(player);
  // dash.speed(500.0)
  REQUIRE(vel.w == Approx(500.0));
  REQUIRE(vel.d == Approx(0.0));
}

TEST_CASE("PlayerMotionSystem - Dash moves in free direction when input given"
) {
  // ダッシュ中にフリー方向の移動入力があればその方向へ移動する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(player, PlayerMotion::Dash{.elapsed = 0.0});

  FrameData frameData{.dt = 0.05};
  frameData.input.moveAxis = Vec2{0.0, 1.0};
  MotionSystem::Update(registry, frameData);

  const auto& vel = registry.get<Velocity>(player);
  REQUIRE(vel.w == Approx(0.0));
  REQUIRE(vel.d == Approx(500.0));
}

TEST_CASE(
    "PlayerMotionSystem - Dash queues dash attack on attack input during "
    "recovery B"
) {
  // 後隙B中（recoveryAEnd 以降）の近接攻撃入力で dashAttackQueued が立つ
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd(0.15) + recoveryASec(0.15) = 0.30 が recoveryAEnd
  registry.replace<Motion>(player, PlayerMotion::Dash{.elapsed = 0.29});

  FrameData frameData{.dt = 0.02};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  REQUIRE(std::get<PlayerMotion::Dash>(motion).dashAttackQueued);
}

TEST_CASE(
    "PlayerMotionSystem - Dash cancels into Dash on dash input during "
    "recovery B"
) {
  // 後隙B中（recoveryAEnd 以降）のダッシュ入力で Dash へキャンセルする
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(player, PlayerMotion::Dash{.elapsed = 0.29});

  FrameData frameData{.dt = 0.02};
  frameData.input.dashDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  REQUIRE(
      std::get<PlayerMotion::Dash>(motion).elapsed == Approx(0.0).margin(0.001)
  );
}

TEST_CASE(
    "PlayerMotionSystem - Dash re-dashes when dashQueued upon entering "
    "recovery B"
) {
  // ダッシュ中に予約された再ダッシュが後隙B入りで発動する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // dashQueued = true（ダッシュ中の入力で予約済み）、後隙Aの末端
  registry.replace<Motion>(
      player, PlayerMotion::Dash{.elapsed = 0.29, .dashQueued = true}
  );

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  REQUIRE(
      std::get<PlayerMotion::Dash>(motion).elapsed == Approx(0.0).margin(0.001)
  );
}

TEST_CASE(
    "PlayerMotionSystem - Dash transitions to Neutral at recovery B end "
    "even when dashAttackQueued"
) {
  // recoveryBEnd を超えたら dashAttackQueued の有無に関わらず Neutral へ戻る
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // recoveryBEnd は windupSec+activeSec+recoveryASec+recoveryBSec = 0.45
  registry.replace<Motion>(
      player, PlayerMotion::Dash{.elapsed = 0.44, .dashAttackQueued = true}
  );

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 recovery dash input cancels into "
    "Dash"
) {
  // Melee（1段目）の後隙中にダッシュ入力があるとダッシュへキャンセルする
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd(0.15) を既に過ぎている状態
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.16, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.01};
  frameData.input.dashDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  REQUIRE_FALSE(std::get<PlayerMotion::Dash>(motion).air);
  REQUIRE(registry.get<Stamina>(player).current == 80);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 1 recovery dash input cancels into "
    "Dash"
) {
  // Melee（2段目）の後隙中にダッシュ入力があるとダッシュへキャンセルする
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // activeEnd(0.20) を既に過ぎている状態
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 1, .elapsed = 0.21, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.01};
  frameData.input.dashDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  REQUIRE(registry.get<Stamina>(player).current == 80);
}

TEST_CASE(
    "PlayerMotionSystem - Melee stage 0 recovery dash input is ignored "
    "without leftover hitbox"
) {
  // Melee 後隙からダッシュへキャンセルする際、ヒットボックスは既に解放済み
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  // activeEnd(0.15) を跨ぐ dt でヒットボックスが解放される
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{.stage = 0, .elapsed = 0.14, .hitboxEntity = hitbox}
  );

  FrameData frameData{.dt = 0.02};
  frameData.input.dashDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  // ヒットボックスは即座に破棄せず、Attack を外して FadeOut
  // へ引き渡す（親からも切り離される）
  REQUIRE(registry.valid(hitbox));
  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));
  REQUIRE(registry.all_of<FadeOut>(hitbox));
  REQUIRE_FALSE(registry.all_of<Hierarchy>(hitbox));
}

TEST_CASE(
    "PlayerMotionSystem - airborne player dash input transitions Neutral to "
    "air Dash"
) {
  // 空中でのダッシュ入力で Neutral から Dash（air=true）へ遷移し、
  // ST が1回分消費される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;

  FrameData frameData{};
  frameData.input.dashDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  const auto& dash = std::get<PlayerMotion::Dash>(motion);
  REQUIRE(dash.air);
  REQUIRE(dash.elapsed == Approx(0.0));

  // dash.staminaCost(20) 分が消費される
  REQUIRE(registry.get<Stamina>(player).current == 80);
}

TEST_CASE(
    "PlayerMotionSystem - airborne dash input is ignored when stamina is "
    "insufficient"
) {
  // ST不足の場合は空中ダッシュ入力を無視し Neutral のまま、ST も減らない
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;
  registry.get<Stamina>(player).current = 10;

  FrameData frameData{};
  frameData.input.dashDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
  REQUIRE(registry.get<Stamina>(player).current == 10);
}

TEST_CASE(
    "PlayerMotionSystem - air Dash grants Invincible during windup and "
    "dash"
) {
  // 構え・ダッシュ中（elapsed < activeEnd）は Invincible が付与される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;  // 空中（接地遷移を避ける）

  // dash.timeline.windupSec(0.0) + activeSec(0.15) = 0.15 未満
  registry.replace<Motion>(
      player, PlayerMotion::Dash{.elapsed = 0.0, .air = true}
  );

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.all_of<Invincible>(player));
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(registry.get<Motion>(player
  )));
}

TEST_CASE(
    "PlayerMotionSystem - air Dash removes Invincible when entering "
    "recovery"
) {
  // activeEnd(0.15) を超えた時点で Invincible が除去される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;  // 空中（接地遷移を避ける）

  registry.emplace<Invincible>(player);
  registry.replace<Motion>(
      player, PlayerMotion::Dash{.elapsed = 0.14, .air = true}
  );

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  REQUIRE_FALSE(registry.all_of<Invincible>(player));
}

TEST_CASE(
    "PlayerMotionSystem - air Dash fixes vertical velocity to zero during "
    "dash movement"
) {
  // ダッシュ移動区間中は垂直速度が 0 に固定される（暫定仕様）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;    // 空中（接地遷移を避ける）
  registry.get<Velocity>(player).h = -200.0;  // 落下中を模した垂直速度
  registry.get<SpriteAnimation>(player).facingRight = true;

  registry.replace<Motion>(
      player, PlayerMotion::Dash{.elapsed = 0.0, .air = true}
  );

  const FrameData frameData{.dt = 0.05};
  MotionSystem::Update(registry, frameData);

  const auto& vel = registry.get<Velocity>(player);
  REQUIRE(vel.h == Approx(0.0));
  // dash.speed(500.0)
  REQUIRE(vel.w == Approx(500.0));
}

TEST_CASE(
    "PlayerMotionSystem - air Dash transitions to Landing when player "
    "lands"
) {
  // 移動・後隙中いずれでも接地したら Landing へ強制遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 0.0;  // 接地
  registry.emplace<Invincible>(player);

  registry.replace<Motion>(
      player, PlayerMotion::Dash{.elapsed = 0.1, .air = true}
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Landing>(motion));
  // 接地遷移時に Invincible が残らないよう除去される
  REQUIRE_FALSE(registry.all_of<Invincible>(player));

  const auto& landing = std::get<PlayerMotion::Landing>(motion);
  REQUIRE(landing.timer == Approx(0.20));
}

TEST_CASE(
    "PlayerMotionSystem - ground Dash does not transition to Landing even "
    "when airborne"
) {
  // 地上 Dash（air=false）は接地遷移を持たないため、空中にいても Landing
  // へは遷移しない
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;  // 空中

  registry.replace<Motion>(
      player, PlayerMotion::Dash{.elapsed = 0.1, .air = false}
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - air Dash transitions to Neutral on recovery "
    "timeout while still airborne"
) {
  // 接地せずに後隙が満了した場合はタイマー満了で Neutral へ戻る
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 500.0;  // 十分な高さで着地しない

  // recoveryBEnd は windupSec+activeSec+recoveryASec+recoveryBSec = 0.45
  registry.replace<Motion>(
      player, PlayerMotion::Dash{.elapsed = 0.44, .air = true}
  );

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - air Dash queues air dash attack on attack input "
    "during recovery B"
) {
  // 後隙B中（recoveryAEnd 以降）の近接攻撃入力で dashAttackQueued が立つ
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;  // 空中（接地遷移を避ける）

  // activeEnd(0.15) + recoveryASec(0.15) = 0.30 が recoveryAEnd
  registry.replace<Motion>(
      player, PlayerMotion::Dash{.elapsed = 0.29, .air = true}
  );

  FrameData frameData{.dt = 0.02};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  REQUIRE(std::get<PlayerMotion::Dash>(motion).dashAttackQueued);
}

TEST_CASE(
    "PlayerMotionSystem - air Dash re-dashes when dashQueued upon entering "
    "recovery B"
) {
  // 空中ダッシュにも再ダッシュ予約（dashQueued）が働く（地上と仕様を揃える）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;  // 空中（接地遷移を避ける）

  registry.replace<Motion>(
      player,
      PlayerMotion::Dash{.elapsed = 0.29, .air = true, .dashQueued = true}
  );

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Dash>(motion));
  const auto& dash = std::get<PlayerMotion::Dash>(motion);
  REQUIRE(dash.air);
  REQUIRE(dash.elapsed == Approx(0.0).margin(0.001));
}

TEST_CASE(
    "PlayerMotionSystem - air Dash transitions to air DashAttack when "
    "dashAttackQueued at recovery A end"
) {
  // 後隙A終了時に予約済みなら空中ダッシュ攻撃（DashAttack、air=true）へ
  // 遷移し、方向を引き継ぐ
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;  // 空中（接地遷移を避ける）

  // recoveryAEnd(0.30) を跨ぐ dt
  registry.replace<Motion>(
      player,
      PlayerMotion::Dash{
          .elapsed = 0.29,
          .air = true,
          .dashAttackQueued = true,
          .lastDashDir = Vec2{0.0, 1.0}
      }
  );

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::DashAttack>(motion));

  const auto& dashAttack = std::get<PlayerMotion::DashAttack>(motion);
  REQUIRE(dashAttack.air);
  REQUIRE(dashAttack.elapsed == Approx(0.0));
  REQUIRE(dashAttack.hitboxEntity == entt::entity{entt::null});
  REQUIRE(dashAttack.dashDir.x == Approx(0.0));
  REQUIRE(dashAttack.dashDir.y == Approx(1.0));

  REQUIRE(registry.get<SpriteAnimation>(player).currentClip == U"dash_attack");
}

TEST_CASE(
    "PlayerMotionSystem - air DashAttack spawns hitbox and moves along w-d "
    "orbit during active frame"
) {
  // 攻撃判定区間でヒットボックスが生成され、w-d 平面上の円軌道で移動する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 100.0;  // 空中（接地遷移を避ける）

  registry.replace<Motion>(
      player,
      PlayerMotion::DashAttack{
          .elapsed = 0.0,
          .air = true,
          .hitboxEntity = entt::null,
          .dashDir = Vec2{1.0, 0.0}
      }
  );

  // windupSec(0.05) を跨ぎ、activeSec(0.20) 中の進行度 0.25 地点まで進める
  const FrameData frameData{.dt = 0.10};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& dashAttack = std::get<PlayerMotion::DashAttack>(motion);
  REQUIRE(dashAttack.hitboxEntity != entt::entity{entt::null});
  REQUIRE(registry.valid(dashAttack.hitboxEntity));
  REQUIRE(registry.all_of<Collider, Attack, LocalOffset>(dashAttack.hitboxEntity
  ));

  // 進行度0.25 → 円上の点（w=0、h=capMidH(40.0)、d=orbitRadius(30.0)）
  const auto& localOffset = registry.get<LocalOffset>(dashAttack.hitboxEntity);
  REQUIRE(localOffset.value.w == Approx(0.0).margin(0.001));
  REQUIRE(localOffset.value.h == Approx(40.0));
  REQUIRE(localOffset.value.d == Approx(30.0));

  const auto& attack = registry.get<Attack>(dashAttack.hitboxEntity);
  REQUIRE(attack.damage == 20);
}

TEST_CASE(
    "PlayerMotionSystem - air DashAttack transitions to Landing when "
    "player lands, destroying leftover hitbox"
) {
  // 後隙中でも接地したら Landing へ強制遷移し、ヒットボックスは破棄される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 0.0;  // 接地

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player,
      PlayerMotion::DashAttack{
          .elapsed = 0.1, .air = true, .hitboxEntity = hitbox
      }
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Landing>(motion));
  // ヒットボックスは即座に破棄せず、Attack を外して FadeOut
  // へ引き渡す（親からも切り離される）
  REQUIRE(registry.valid(hitbox));
  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));
  REQUIRE(registry.all_of<FadeOut>(hitbox));
  REQUIRE_FALSE(registry.all_of<Hierarchy>(hitbox));

  const auto& landing = std::get<PlayerMotion::Landing>(motion);
  REQUIRE(landing.timer == Approx(0.20));
}

TEST_CASE(
    "PlayerMotionSystem - ground DashAttack does not transition to Landing "
    "even when airborne"
) {
  // 地上 DashAttack（air=false）は接地遷移を持たない
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h =
      0.0;  // 接地していても air=false なら無関係

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player,
      PlayerMotion::DashAttack{
          .elapsed = 0.1, .air = false, .hitboxEntity = hitbox
      }
  );

  const FrameData frameData{.dt = 0.01};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::DashAttack>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - air DashAttack transitions to Neutral on "
    "recovery timeout while still airborne"
) {
  // 接地せずに後隙が満了した場合はタイマー満了で Neutral へ戻る
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 500.0;  // 十分な高さで着地しない

  // recoveryEnd は windupSec+activeSec+recoveryASec+recoveryBSec(0)
  // = 0.05+0.20+0.20 = 0.45
  registry.replace<Motion>(
      player,
      PlayerMotion::DashAttack{
          .elapsed = 0.44, .air = true, .hitboxEntity = entt::null
      }
  );

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE(
    "PlayerMotionSystem - Hitstop freezes Melee elapsed and keeps the same "
    "hitboxEntity"
) {
  // Hitstop 中は dt = 0 で Tick されるため、elapsed もヒットボックスの再生成も
  // 起きない
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.emplace<Hitstop>(player, Hitstop{.remaining = 0.05});

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{.stage = 0, .elapsed = 0.06, .hitboxEntity = hitbox}
  );

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  REQUIRE(melee.elapsed == Approx(0.06));
  REQUIRE(melee.hitboxEntity == hitbox);
  REQUIRE(registry.valid(hitbox));
}

TEST_CASE(
    "PlayerMotionSystem - Hitstop still lets attackDown set comboQueued "
    "during active frame"
) {
  // dt = 0 でも input は読まれるため、停止中の攻撃入力は予約として拾える
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.emplace<Hitstop>(player, Hitstop{.remaining = 0.05});

  // windupSec(0.05) を超え activeEnd(0.15) 未満（active区間）
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0, .elapsed = 0.10, .hitboxEntity = entt::null
      }
  );

  FrameData frameData{.dt = 0.1};
  frameData.input.attackDown = true;
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  // elapsed は凍結されたまま
  REQUIRE(melee.elapsed == Approx(0.10));
  REQUIRE(melee.comboQueued);
}

TEST_CASE(
    "PlayerMotionSystem - Melee resumes and transitions to next stage once "
    "Hitstop clears"
) {
  // 停止中は次段への遷移が起きず、Hitstop 除去後の後隙Bで初めて遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.emplace<Hitstop>(player, Hitstop{.remaining = 0.05});

  // activeEnd(0.15) 手前、次段への予約は既に立っている
  registry.replace<Motion>(
      player,
      PlayerMotion::Melee{
          .stage = 0,
          .elapsed = 0.14,
          .hitboxEntity = entt::null,
          .comboQueued = true
      }
  );

  // 停止中：dt を与えても elapsed は凍結されたまま、まだ Melee stage 0
  MotionSystem::Update(registry, FrameData{.dt = 0.1});
  {
    const auto& motion = registry.get<Motion>(player);
    const auto& melee = std::get<PlayerMotion::Melee>(motion);
    REQUIRE(melee.stage == 0);
    REQUIRE(melee.elapsed == Approx(0.14));
  }

  // 停止解除後：activeEnd を跨いで後隙Bへ入り、予約済みの次段へ遷移する
  registry.remove<Hitstop>(player);
  MotionSystem::Update(registry, FrameData{.dt = 0.02});

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).stage == 1);
}

#endif
