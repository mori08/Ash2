#include "Phase/PlayerTestPhase.hpp"

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Gravity.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hitstop.hpp"
#include "Component/Hp.hpp"
#include "Component/Motion.hpp"
#include "Component/Name.hpp"
#include "Component/Player.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Stagger.hpp"
#include "Component/Stamina.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/AnimationSystem.hpp"
#include "System/AttachmentSystem.hpp"
#include "System/GravitySystem.hpp"
#include "System/HitSystem.hpp"
#include "System/HitstopSystem.hpp"
#include "System/MotionSystem.hpp"
#include "System/MovementSystem.hpp"
#include "System/ProjectileSystem.hpp"
#include "System/StaggerSystem.hpp"

constexpr double KDummyPosW = 150.0;
constexpr s3d::SizeF KDummySize = {60.0, 80.0};
constexpr s3d::ColorF KDummyColor = {0.8, 0.2, 0.2};
constexpr double KDummyCapRadius = 30.0;
constexpr double KDummyCapHeight = 80.0;
constexpr int KDummyMaxHp = 100;
constexpr int KPlayerMaxHp = 100;
constexpr int KPlayerMaxStamina = 100;
/// 暫定のひるみ時間（秒）。本格的な数値調整は #132/#134 で行う
constexpr double KStaggerSec = 0.15;

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

  HitstopSystem::Update(registry, dt);
  MotionSystem::Update(registry, frameData);
  MovementSystem::Update(registry, dt);
  GravitySystem::Update(registry, dt);
  AttachmentSystem::UpdateTransform(registry);

  const auto hits = HitSystem::Update(registry);
  applyHitReactions(registry, hits);
  ProjectileSystem::Update(registry);

  StaggerSystem::Update(registry, dt);
  AnimationSystem::Update(registry, dt);

  if (frameData.input.reloadConfig) {
    reloadPlayer(registry);
  }

  if (s3d::KeyEscape.down()) {
    return PhaseCommand::Pop();
  }

  return PhaseCommand::None();
}

void PlayerTestPhase::applyHitReactions(entt::registry& registry,
                                        const s3d::Array<HitPair>& hits) {
  for (const auto& hit : hits) {
    const auto& attack = registry.get<Attack>(hit.attacker);
    if (attack.hitstopSec <= 0.0) continue;

    // ヒットボックスの親（プレイヤー本体）にヒットストップを付与する
    auto attackerOwner = hit.attacker;
    if (const auto* hierarchy = registry.try_get<Hierarchy>(hit.attacker);
        hierarchy != nullptr && hierarchy->parent() != entt::null) {
      attackerOwner = hierarchy->parent();
    }
    registry.emplace_or_replace<Hitstop>(
        attackerOwner, Hitstop{.remaining = attack.hitstopSec});
    registry.emplace_or_replace<Hitstop>(
        hit.target, Hitstop{.remaining = attack.hitstopSec});

    // ひるみリアクション（最小実装：本格的な状態機械は #134 のスコープ）
    if (auto* drawable = registry.try_get<Drawable>(hit.target);
        drawable != nullptr) {
      if (auto* rect = std::get_if<RectDrawable>(drawable); rect != nullptr) {
        // 既にひるみ中なら originalSize を引き継ぎ、縮小済みサイズを
        // originalSize として上書きしてしまうのを防ぐ
        s3d::SizeF originalSize = rect->size;
        if (const auto* existing = registry.try_get<Stagger>(hit.target);
            existing != nullptr) {
          originalSize = existing->originalSize;
          rect->size = originalSize;
        }
        registry.emplace_or_replace<Stagger>(
            hit.target, Stagger{.remaining = KStaggerSec,
                                .duration = KStaggerSec,
                                .originalSize = originalSize});
      }
    }
  }
}

void PlayerTestPhase::reloadPlayer(entt::registry& registry) {
  onBeforePop(registry);
  onAfterPush(registry);
}

void PlayerTestPhase::onBeforePop(entt::registry& registry) {
  // 攻撃判定エンティティ（PlayerMotion::Melee1/Melee2.hitboxEntity）は
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
