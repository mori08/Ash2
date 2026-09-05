#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/AttackOrb.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Enemy.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/FadeOut.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hitstop.hpp"
#include "Component/Hp.hpp"
#include "Component/Invincible.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/Player.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/ReactionLevel.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "System/HitReactionSystem.hpp"
#include "System/HitSystem.hpp"

namespace {

/// @brief テスト用の registry.ctx() セットアップ（EnemyConfig + PlayerConfig）
void SetupContext(entt::registry& registry) {
  registry.ctx().emplace<EnemyConfig>(EnemyConfig{
      .maxHp = 100,
      .size = {60.0, 80.0},
      .capsuleRadius = 30.0,
      .capsuleHeight = 80.0,
      .spawnW = 150.0,
      .staggerSec = 0.15,
      .repelSpeed = 250.0,
      .repelSec = 0.20,
      .blowSpeedW = 300.0,
      .blowSpeedH = 300.0,
      .knockbackSec = 1.00,
      .defeatedSec = 0.50,
      .respawnSec = 1.00,
  });

  registry.ctx().emplace<PlayerConfig>(PlayerConfig{
      .capsuleRadius = 20.0,
      .capsuleHeight = 60.0,
      .dash = {.staminaCost = 20},
      .attackEffect = {.fadeSec = 0.30},
      .damage = {
          .staggerSec = 0.20,
          .knockbackSpeedW = 250.0,
          .knockbackSpeedH = 300.0,
          .downSec = 0.50,
          .getUpSec = 0.30
      },
  });
}

/// @brief 攻撃側本体エンティティ（WorldPos のみ）を生成する
entt::entity MakeAttackerOwner(entt::registry& registry, double ownerW) {
  const auto owner = registry.create();
  registry.emplace<WorldPos>(owner, WorldPos{.w = ownerW});
  return owner;
}

/// @brief 攻撃側本体エンティティを生成し、対応する HitEvent を組み立てる
HitEvent MakeHitEvent(
    entt::registry& registry, double ownerW, entt::entity target,
    ReactionLevel reaction, double hitstopSec = 0.05
) {
  return HitEvent{
      .target = target,
      .attackerOwner = MakeAttackerOwner(registry, ownerW),
      .hitstopSec = hitstopSec,
      .reaction = reaction,
  };
}

/// @brief テスト用の敵（被弾側）エンティティを生成する
entt::entity MakeTarget(entt::registry& registry, double targetW) {
  const auto target = registry.create();
  registry.emplace<Enemy>(target);
  registry.emplace<WorldPos>(target, WorldPos{.w = targetW});
  registry.emplace<Velocity>(target);
  registry.emplace<Collider>(target);
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});
  registry.emplace<EnemyMotion::Variant>(target, EnemyMotion::Idle{});
  return target;
}

/// @brief テスト用のプレイヤー（被弾側）エンティティを生成する
entt::entity MakePlayerTarget(entt::registry& registry, double targetW) {
  const auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player, WorldPos{.w = targetW});
  registry.emplace<Velocity>(player);
  registry.emplace<SpriteAnimation>(
      player, SpriteAnimation{.dataKey = U"player", .currentClip = U"idle"}
  );
  registry.emplace<PlayerMotion::Variant>(player, PlayerMotion::Neutral{});
  return player;
}

}  // namespace

TEST_CASE("HitReactionSystem - Stagger reaction transitions Enemy to Stagger") {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Stagger)}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Stagger>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
}

TEST_CASE(
    "HitReactionSystem - Repel reaction transitions Enemy to Repel and sets "
    "velocity"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Repel)}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Repel>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
  REQUIRE(registry.get<Velocity>(target).w == Approx(250.0));
}

TEST_CASE(
    "HitReactionSystem - Blow reaction transitions Enemy to Knockback and "
    "sets velocity"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Blow)}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
  REQUIRE(registry.get<Velocity>(target).w == Approx(300.0));
  REQUIRE(registry.get<Velocity>(target).h == Approx(300.0));
}

TEST_CASE("HitReactionSystem - None reaction leaves Enemy in Idle") {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::None)}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
}

TEST_CASE(
    "HitReactionSystem - Hp depletion forces Defeated regardless of "
    "reaction"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);
  registry.get<Hp>(target).current = 0;

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Stagger)}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Defeated>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
  REQUIRE_FALSE(registry.all_of<Collider>(target));
  REQUIRE_FALSE(registry.all_of<Hp>(target));
}

TEST_CASE(
    "HitReactionSystem - hit from an owner to the left pushes target to the "
    "right (positive vel.w)"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, /*targetW=*/50.0);

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(registry, /*ownerW=*/0.0, target, ReactionLevel::Repel)}
  );

  REQUIRE(registry.get<Velocity>(target).w > 0.0);
}

TEST_CASE(
    "HitReactionSystem - hit from an owner to the right pushes target to "
    "the left (negative vel.w)"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, /*targetW=*/50.0);

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(registry, /*ownerW=*/100.0, target, ReactionLevel::Repel)}
  );

  REQUIRE(registry.get<Velocity>(target).w < 0.0);
}

TEST_CASE(
    "HitReactionSystem - hitstopSec <= 0 skips hitstop but still applies "
    "reaction"
) {
  // 弾（Ranged）は hitstopSec 0 で作られるが、ひるみ・撃破自体は起きる
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(
          registry, 0.0, target, ReactionLevel::Blow, /*hitstopSec=*/0.0
      )}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
  REQUIRE_FALSE(registry.all_of<Hitstop>(target));
}

TEST_CASE(
    "HitReactionSystem - hitstopSec <= 0 still forces Defeated on Hp "
    "depletion"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);
  registry.get<Hp>(target).current = 0;

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(
          registry, 0.0, target, ReactionLevel::None, /*hitstopSec=*/0.0
      )}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Defeated>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
  REQUIRE_FALSE(registry.all_of<Hitstop>(target));
}

TEST_CASE(
    "HitReactionSystem - Stagger hit while Repel resets Velocity.w to "
    "zero"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);
  registry.replace<EnemyMotion::Variant>(
      target, EnemyMotion::Repel{.remaining = 0.1}
  );
  registry.get<Velocity>(target).w = 250.0;

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Stagger)}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Stagger>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
  REQUIRE(registry.get<Velocity>(target).w == Approx(0.0));
}

TEST_CASE(
    "HitReactionSystem - Blow hit while Stagger restores RectDrawable "
    "size"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);
  registry.replace<EnemyMotion::Variant>(
      target, EnemyMotion::Stagger{.remaining = 0.05}
  );
  registry.emplace<Drawable>(target, RectDrawable{.size = {60.0, 40.0}});

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Blow)}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
  const auto& rect = std::get<RectDrawable>(registry.get<Drawable>(target));
  REQUIRE(rect.size.y == Approx(80.0));
}

TEST_CASE(
    "HitReactionSystem - None reaction while Knockback keeps Velocity "
    "untouched"
) {
  // 弾（reaction 既定の None）が当たっても、進行中の Knockback は乱さない
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);
  registry.replace<EnemyMotion::Variant>(
      target, EnemyMotion::Knockback{.remaining = 0.5}
  );
  registry.get<Velocity>(target).w = 300.0;
  registry.get<Velocity>(target).h = 300.0;

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(
          registry, 0.0, target, ReactionLevel::None, /*hitstopSec=*/0.0
      )}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<EnemyMotion::Variant>(target)
      )
  );
  REQUIRE(registry.get<Velocity>(target).w == Approx(300.0));
  REQUIRE(registry.get<Velocity>(target).h == Approx(300.0));
}

TEST_CASE(
    "HitReactionSystem - grants Hitstop to the attacker's owner and the "
    "target"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto owner = MakeAttackerOwner(registry, 0.0);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry,
      {HitEvent{
          .target = target,
          .attackerOwner = owner,
          .hitstopSec = 0.1,
          .reaction = ReactionLevel::Stagger
      }}
  );

  REQUIRE(registry.all_of<Hitstop>(owner));
  REQUIRE(registry.all_of<Hitstop>(target));
}

TEST_CASE(
    "HitReactionSystem - overlapping Hitstop keeps the longer remaining "
    "time"
) {
  // 停止中に短いヒットが重なっても、長い方の残り時間を維持する
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(
          registry, 0.0, target, ReactionLevel::Stagger, /*hitstopSec=*/0.2
      )}
  );
  REQUIRE(registry.get<Hitstop>(target).remaining == Approx(0.2));

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(
          registry, 0.0, target, ReactionLevel::Stagger, /*hitstopSec=*/0.05
      )}
  );
  REQUIRE(registry.get<Hitstop>(target).remaining == Approx(0.2));
}

TEST_CASE(
    "HitReactionSystem - overlapping Hitstop extends to a longer new "
    "remaining time"
) {
  // 停止中により長いヒットが重なったら、その長い方へ更新する
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(
          registry, 0.0, target, ReactionLevel::Stagger, /*hitstopSec=*/0.05
      )}
  );
  REQUIRE(registry.get<Hitstop>(target).remaining == Approx(0.05));

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(
          registry, 0.0, target, ReactionLevel::Stagger, /*hitstopSec=*/0.2
      )}
  );
  REQUIRE(registry.get<Hitstop>(target).remaining == Approx(0.2));
}

TEST_CASE(
    "HitReactionSystem - target lacking any motion variant only receives "
    "Hitstop, no reaction applied"
) {
  // EnemyMotion::Variant/PlayerMotion::Variant のどちらも持たない被弾側は、
  // どちらの分岐にも入らずリアクション適用の対象外になる
  entt::registry registry;
  SetupContext(registry);

  const auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 50.0});

  HitReactionSystem::Apply(
      registry,
      {MakeHitEvent(
          registry, 0.0, target, ReactionLevel::Blow, /*hitstopSec=*/0.1
      )}
  );

  REQUIRE(registry.all_of<Hitstop>(target));
  REQUIRE_FALSE(registry.all_of<EnemyMotion::Variant>(target));
  REQUIRE_FALSE(registry.all_of<PlayerMotion::Variant>(target));
}

TEST_CASE(
    "HitReactionSystem - player Stagger reaction (Lv1) transitions "
    "PlayerMotion to Stagger"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakePlayerTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Stagger)}
  );

  REQUIRE(
      std::holds_alternative<PlayerMotion::Stagger>(
          registry.get<PlayerMotion::Variant>(target)
      )
  );
}

TEST_CASE(
    "HitReactionSystem - player Repel reaction (Lv2) transitions "
    "PlayerMotion to Stagger"
) {
  // battle_design.md の「Lv1 と Lv2 は同じ仰け反りに寄せる」に従い、
  // Repel も Stagger に丸める
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakePlayerTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Repel)}
  );

  REQUIRE(
      std::holds_alternative<PlayerMotion::Stagger>(
          registry.get<PlayerMotion::Variant>(target)
      )
  );
}

TEST_CASE(
    "HitReactionSystem - player Blow reaction (Lv3) transitions "
    "PlayerMotion to Knockback and sets velocity and Invincible"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakePlayerTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Blow)}
  );

  REQUIRE(
      std::holds_alternative<PlayerMotion::Knockback>(
          registry.get<PlayerMotion::Variant>(target)
      )
  );
  // 攻撃側本体（w=0.0）より右（w=50.0）にいるので正方向へ吹き飛ぶ
  REQUIRE(registry.get<Velocity>(target).w == Approx(250.0));
  REQUIRE(registry.get<Velocity>(target).h == Approx(300.0));
  REQUIRE(registry.all_of<Invincible>(target));
}

TEST_CASE(
    "HitReactionSystem - player None reaction leaves PlayerMotion in Neutral"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakePlayerTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::None)}
  );

  REQUIRE(
      std::holds_alternative<PlayerMotion::Neutral>(
          registry.get<PlayerMotion::Variant>(target)
      )
  );
}

TEST_CASE(
    "HitReactionSystem - player hit while attacking releases hitboxEntity"
) {
  // 攻撃中に被弾したとき、上書き前の MeleeChain が持つヒットボックスが
  // ReleaseAttackHitbox で解放される（後始末。ARCHITECTURE.md の
  // 「例外：外部要因による強制遷移」を参照）
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakePlayerTarget(registry, 50.0);

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.emplace<AttackOrb>(hitbox);
  Hierarchy::Attach(registry, target, hitbox);
  registry.replace<PlayerMotion::Variant>(
      target,
      PlayerMotion::MeleeChain{
          .stage = 0, .elapsed = 0.10, .hitboxEntity = hitbox
      }
  );

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Stagger)}
  );

  REQUIRE(
      std::holds_alternative<PlayerMotion::Stagger>(
          registry.get<PlayerMotion::Variant>(target)
      )
  );
  // ヒットボックスは即座に破棄せず、Attack を外して FadeOut
  // へ引き渡す（親からも切り離される）
  REQUIRE(registry.valid(hitbox));
  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));
  REQUIRE(registry.all_of<FadeOut>(hitbox));
  REQUIRE(registry.get<Hierarchy>(hitbox).parent() == entt::entity{entt::null});
  REQUIRE_FALSE(registry.all_of<LocalOffset>(hitbox));
}

TEST_CASE(
    "HitReactionSystem - a same-frame hit against Enemy is still processed "
    "after an earlier Player hit releases the Player's own hitbox"
) {
  // 修正5回目の回帰テスト。同一フレームに「敵→プレイヤー」「プレイヤーの
  // ヒットボックス→敵」の順でヒットが並ぶと、1件目（プレイヤーの被弾）の
  // MakeDamaged → ReleaseAttackHitbox がプレイヤー自身の近接ヒットボックス
  // から Attack を外す。HitEvent はヒットボックスのエンティティ自体を
  // 保持しないため、2件目（敵への攻撃）も正しく処理される
  entt::registry registry;
  SetupContext(registry);

  const auto enemyOwner = MakeAttackerOwner(registry, 0.0);
  const auto player = MakePlayerTarget(registry, 50.0);
  const auto enemy = MakeTarget(registry, 100.0);

  // プレイヤーが攻撃中（MeleeChain のヒットボックスが Attack を保持）
  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.emplace<Attack>(
      hitbox,
      Attack{.damage = 10, .hitstopSec = 0.05, .reaction = ReactionLevel::Blow}
  );
  registry.emplace<AttackOrb>(hitbox);
  Hierarchy::Attach(registry, player, hitbox);
  registry.replace<PlayerMotion::Variant>(
      player,
      PlayerMotion::MeleeChain{
          .stage = 0, .elapsed = 0.10, .hitboxEntity = hitbox
      }
  );

  HitReactionSystem::Apply(
      registry,
      {HitEvent{
           .target = player,
           .attackerOwner = enemyOwner,
           .hitstopSec = 0.05,
           .reaction = ReactionLevel::Stagger
       },
       HitEvent{
           .target = enemy,
           .attackerOwner = player,
           .hitstopSec = 0.05,
           .reaction = ReactionLevel::Blow
       }}
  );

  // 1件目：プレイヤーは Stagger へ遷移し、ヒットボックスの Attack が外れる
  REQUIRE(
      std::holds_alternative<PlayerMotion::Stagger>(
          registry.get<PlayerMotion::Variant>(player)
      )
  );
  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));

  // 2件目：先行するヒットボックス解放の影響を受けず、敵側のリアクションが
  // 適用される
  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(registry.get<Velocity>(enemy).w > 0.0);
}

TEST_CASE(
    "HitReactionSystem - player hit during MeleeFinisher releases hitbox "
    "and light orbs together"
) {
  // MeleeFinisher の光2つを含む状態から被弾すると、hitboxEntity・
  // lightEntities の珠がすべて所有者の子から外れ、Attack を失う
  // （ReleaseAttackOrbs が所有者の子を AttackOrb タグで一括解放するため）
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakePlayerTarget(registry, 50.0);

  const auto hitbox = registry.create();
  registry.emplace<LocalOffset>(hitbox);
  registry.emplace<Collider>(hitbox);
  registry.emplace<Attack>(
      hitbox,
      Attack{.damage = 10, .hitstopSec = 0.05, .reaction = ReactionLevel::Blow}
  );
  registry.emplace<AttackOrb>(hitbox);
  Hierarchy::Attach(registry, target, hitbox);

  Array<entt::entity> lights;
  for (int32 i = 0; i < 2; ++i) {
    const auto light = registry.create();
    registry.emplace<LocalOffset>(light);
    registry.emplace<AttackOrb>(light, AttackOrb{.index = i});
    Hierarchy::Attach(registry, target, light);
    lights.push_back(light);
  }

  registry.replace<PlayerMotion::Variant>(
      target,
      PlayerMotion::MeleeFinisher{
          .elapsed = 0.10, .hitboxEntity = hitbox, .lightEntities = lights
      }
  );

  HitReactionSystem::Apply(
      registry, {MakeHitEvent(registry, 0.0, target, ReactionLevel::Stagger)}
  );

  REQUIRE(
      std::holds_alternative<PlayerMotion::Stagger>(
          registry.get<PlayerMotion::Variant>(target)
      )
  );

  REQUIRE_FALSE(registry.all_of<Attack>(hitbox));
  REQUIRE(registry.get<Hierarchy>(hitbox).parent() == entt::entity{entt::null});
  for (const auto light : lights) {
    REQUIRE(
        registry.get<Hierarchy>(light).parent() == entt::entity{entt::null}
    );
  }
}

#endif
