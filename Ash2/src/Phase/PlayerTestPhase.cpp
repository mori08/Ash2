#include "Phase/PlayerTestPhase.hpp"

#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Gravity.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hp.hpp"
#include "Component/Name.hpp"
#include "Component/NeutralState.hpp"
#include "Component/Player.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Stamina.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/AnimationSystem.hpp"
#include "System/AttackStateSystem.hpp"
#include "System/GravitySystem.hpp"
#include "System/HitSystem.hpp"
#include "System/MovementSystem.hpp"
#include "System/NeutralStateSystem.hpp"
#include "System/PlayerMovementSystem.hpp"
#include "System/ProjectileSystem.hpp"

constexpr double KDummyPosW = 150.0;
constexpr s3d::SizeF KDummySize = {60.0, 80.0};
constexpr s3d::ColorF KDummyColor = {0.8, 0.2, 0.2};
constexpr double KDummyCapRadius = 30.0;
constexpr double KDummyCapHeight = 80.0;
constexpr int KDummyMaxHp = 100;
constexpr int KPlayerMaxHp = 100;
constexpr int KPlayerMaxStamina = 100;

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
  registry.emplace<NeutralState>(m_playerRoot);
  AnimationSystem::Update(registry, 0.0);

  // ダミーターゲット（縦カプセル: 足元〜高さ80、半径30）
  m_dummyTarget = registry.create();
  registry.emplace<WorldPos>(m_dummyTarget, WorldPos{.w = KDummyPosW});
  registry.emplace<Drawable>(m_dummyTarget,
                             RectDrawable{.size = KDummySize,
                                          .color = KDummyColor,
                                          .anchor = DrawAnchor::BottomCenter});
  registry.emplace<Collider>(m_dummyTarget,
                             Collider{
                                 .segmentStart = Vec3{0.0, 0.0, 0.0},
                                 .segmentEnd = Vec3{0.0, KDummyCapHeight, 0.0},
                                 .radius = KDummyCapRadius,
                             });
  registry.emplace<Hp>(m_dummyTarget,
                       Hp{.max = KDummyMaxHp, .current = KDummyMaxHp});
}

IPhase::PhaseCommand PlayerTestPhase::update(entt::registry& registry,
                                             const FrameData& frameData) {
  const double dt = frameData.dt;

  AttackStateSystem::Update(registry, frameData);
  PlayerMovementSystem::Update(registry, frameData);
  MovementSystem::Update(registry, dt);
  GravitySystem::Update(registry, dt);
  NeutralStateSystem::Update(registry, frameData);

  HitSystem::Update(registry);
  ProjectileSystem::Update(registry);

  AnimationSystem::Update(registry, dt);

  if (frameData.input.reloadConfig) {
    reloadPlayer(registry);
  }

  if (s3d::KeyEscape.down()) {
    return PhaseCommand::Pop();
  }

  return PhaseCommand::None();
}

void PlayerTestPhase::reloadPlayer(entt::registry& registry) {
  onBeforePop(registry);
  onAfterPush(registry);
}

void PlayerTestPhase::onBeforePop(entt::registry& registry) {
  // 攻撃判定エンティティ（AttackState.entity）は m_playerRoot
  // の子孫なので DestroyWithChildren で連動して破棄される
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
