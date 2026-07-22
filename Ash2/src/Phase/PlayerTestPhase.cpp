#include "Phase/PlayerTestPhase.hpp"

#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Enemy.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/Gravity.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hp.hpp"
#include "Component/Motion.hpp"
#include "Component/Name.hpp"
#include "Component/Player.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Stamina.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/AnimationSystem.hpp"
#include "System/AttachmentSystem.hpp"
#include "System/EnemySystem.hpp"
#include "System/GravitySystem.hpp"
#include "System/HitReactionSystem.hpp"
#include "System/HitSystem.hpp"
#include "System/HitstopSystem.hpp"
#include "System/MotionSystem.hpp"
#include "System/MovementSystem.hpp"
#include "System/ProjectileSystem.hpp"
#include "System/StaminaSystem.hpp"

constexpr ColorF KDummyColor = {0.8, 0.2, 0.2};
constexpr int32 KPlayerMaxHp = 100;
constexpr int32 KPlayerMaxStamina = 100;

void PlayerTestPhase::onAfterPush(entt::registry& registry) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();

  m_playerRoot = registry.create();
  registry.emplace<Player>(m_playerRoot);
  registry.emplace<WorldPos>(m_playerRoot);
  registry.emplace<Velocity>(m_playerRoot);
  registry.emplace<Gravity>(m_playerRoot, Gravity{.accel = cfg.gravity});
  registry.emplace<Name>(m_playerRoot, Name{U"player"});
  registry.emplace<Drawable>(
      m_playerRoot, TextureDrawable{.anchor = DrawAnchor::BottomCenter});
  registry.emplace<SpriteAnimation>(
      m_playerRoot,
      SpriteAnimation{.dataKey = U"player", .currentClip = U"idle"});
  registry.emplace<Hp>(m_playerRoot,
                       Hp{.max = KPlayerMaxHp, .current = KPlayerMaxHp});
  registry.emplace<Stamina>(
      m_playerRoot,
      Stamina{.max = KPlayerMaxStamina, .current = KPlayerMaxStamina});
  registry.emplace<Motion>(m_playerRoot, PlayerMotion::Neutral{});
  AnimationSystem::Update(registry, 0.0);

  m_dummyTarget = spawnEnemy(registry);
}

entt::entity PlayerTestPhase::spawnEnemy(entt::registry& registry) {
  const auto& enemyCfg = registry.ctx().get<EnemyConfig>();
  const auto& playerCfg = registry.ctx().get<PlayerConfig>();

  const auto enemy = registry.create();
  registry.emplace<Enemy>(enemy);
  registry.emplace<WorldPos>(enemy, WorldPos{.w = enemyCfg.spawnW});
  registry.emplace<Velocity>(enemy);
  // Knockback の放物線は GravitySystem に任せるため、プレイヤーと同じ重力
  // 加速度を与える（EnemyConfig は専用の重力値を持たない）
  registry.emplace<Gravity>(enemy, Gravity{.accel = playerCfg.gravity});
  registry.emplace<Motion>(enemy, EnemyMotion::Idle{});
  registry.emplace<Drawable>(enemy,
                             RectDrawable{.size = enemyCfg.size,
                                          .color = KDummyColor,
                                          .anchor = DrawAnchor::BottomCenter});
  registry.emplace<Collider>(
      enemy, Collider{.segmentStart = Vec3{0.0, 0.0, 0.0},
                      .segmentEnd = Vec3{0.0, enemyCfg.capsuleHeight, 0.0},
                      .radius = enemyCfg.capsuleRadius});
  registry.emplace<Hp>(enemy,
                       Hp{.max = enemyCfg.maxHp, .current = enemyCfg.maxHp});
  return enemy;
}

IPhase::PhaseCommand PlayerTestPhase::update(entt::registry& registry,
                                             const FrameData& frameData) {
  const double dt = frameData.dt;

  HitstopSystem::Update(registry, dt);
  MotionSystem::Update(registry, frameData);
  StaminaSystem::Update(registry, dt);
  MovementSystem::Update(registry, dt);
  GravitySystem::Update(registry, dt);
  AttachmentSystem::UpdateTransform(registry);

  const auto hits = HitSystem::Update(registry);
  HitReactionSystem::Apply(registry, hits);
  ProjectileSystem::Update(registry);
  EnemySystem::Update(registry);

  AnimationSystem::Update(registry, dt);

  // 敵が撃破され破棄されたら respawnSec 後に再生成する
  if (m_dummyTarget != entt::null && !registry.valid(m_dummyTarget)) {
    m_dummyTarget = entt::null;
    m_respawnTimer = registry.ctx().get<EnemyConfig>().respawnSec;
  } else if (m_dummyTarget == entt::null) {
    m_respawnTimer -= dt;
    if (m_respawnTimer <= 0.0) {
      m_dummyTarget = spawnEnemy(registry);
    }
  }

  if (frameData.input.reloadConfig) {
    reloadPlayer(registry);
  }

  if (KeyEscape.down()) {
    return PhaseCommand::Pop();
  }

  return PhaseCommand::None();
}

void PlayerTestPhase::reloadPlayer(entt::registry& registry) {
  onBeforePop(registry);
  onAfterPush(registry);
}

void PlayerTestPhase::onBeforePop(entt::registry& registry) {
  // 攻撃判定エンティティ（PlayerMotion::Melee.hitboxEntity 等）は
  // m_playerRoot の子孫なので DestroyWithChildren で連動して破棄される
  if (m_playerRoot != entt::null) {
    Hierarchy::DestroyWithChildren(registry, m_playerRoot);
    m_playerRoot = entt::null;
  }
  if (m_dummyTarget != entt::null && registry.valid(m_dummyTarget)) {
    registry.destroy(m_dummyTarget);
    m_dummyTarget = entt::null;
  }

  // 弾は独立エンティティ（m_playerRoot の子孫ではない）なので、
  // Projectile タグで検索して個別に破棄する
  for (const auto entity : registry.view<Projectile>()) {
    registry.destroy(entity);
  }
}
