#include "Phase/PlayerTestPhase.hpp"

#include "Component/AnimationData.hpp"
#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hp.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/Name.hpp"
#include "Component/Player.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/AnimationSystem.hpp"
#include "System/HitSystem.hpp"
#include "System/PlayerMovementSystem.hpp"

constexpr double KDummyPosW = 150.0;
constexpr s3d::SizeF KDummySize = {60.0, 80.0};
constexpr s3d::ColorF KDummyColor = {0.8, 0.2, 0.2};
constexpr double KDummyCapRadius = 30.0;
constexpr double KDummyCapHeight = 80.0;
constexpr int KDummyMaxHp = 100;

void PlayerTestPhase::onAfterPush(entt::registry& registry) {
  m_playerRoot = registry.create();
  registry.emplace<Player>(m_playerRoot);
  registry.emplace<WorldPos>(m_playerRoot);
  registry.emplace<Velocity>(m_playerRoot);
  registry.emplace<Name>(m_playerRoot, Name{U"player"});
  registry.emplace<Drawable>(m_playerRoot, TextureDrawable{});
  registry.emplace<SpriteAnimation>(
      m_playerRoot,
      SpriteAnimation{.dataKey = U"player", .currentClip = U"idle"});
  AnimationSystem::Update(registry, 0.0);

  // ダミーターゲット（縦カプセル: 足元〜高さ80、半径30）
  m_dummyTarget = registry.create();
  registry.emplace<WorldPos>(m_dummyTarget, WorldPos{.w = KDummyPosW});
  registry.emplace<Drawable>(
      m_dummyTarget, RectDrawable{.size = KDummySize, .color = KDummyColor});
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
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& input = frameData.input;
  const double dt = frameData.dt;

  // AnimationDataRegistry からクリップ期間を取得
  const auto& animReg = registry.ctx().get<AnimationDataRegistry>();
  const auto& playerData = animReg.at(U"player");
  const auto getClipDuration = [&](const s3d::String& clipName) -> double {
    const auto it = playerData.clips.find(clipName);
    if (it == playerData.clips.end()) return 0.0;
    return static_cast<double>(it->second.count) / it->second.speed;
  };

  // 攻撃タイマー更新（0以下で攻撃終了）
  if (!m_attackClip.empty()) {
    m_attackTimer -= dt;
    if (m_attackTimer <= 0.0) {
      m_attackClip = U"";
      if (m_attackEntity != entt::null) {
        Hierarchy::DestroyWithChildren(registry, m_attackEntity);
        m_attackEntity = entt::null;
      }
    }
  }

  PlayerMovementSystem::Update(registry, frameData, m_attackClip);

  const bool isAttacking = !m_attackClip.empty();

  auto view = registry.view<Player, WorldPos, Velocity, SpriteAnimation>();
  for (const auto& [entity, pos, vel, anim] : view.each()) {
    const bool onGround = pos.isOnGround();

    // 攻撃入力チェック（攻撃中でない、かつ地上にいるときのみ）
    if (!isAttacking && onGround) {
      if (input.attackDown) {
        m_attackClip = U"attack";
        m_attackTimer = getClipDuration(U"attack");

        const double sign = anim.facingRight ? 1.0 : -1.0;
        m_attackEntity = registry.create();
        registry.emplace<WorldPos>(m_attackEntity,
                                   registry.get<WorldPos>(m_playerRoot));
        registry.emplace<LocalOffset>(m_attackEntity, LocalOffset{});
        Hierarchy::Attach(registry, m_playerRoot, m_attackEntity);
        registry.emplace<Collider>(
            m_attackEntity,
            Collider{
                .segmentStart = Vec3{0.0, cfg.melee.capMidH, 0.0},
                .segmentEnd =
                    Vec3{sign * cfg.melee.reach, cfg.melee.capMidH, 0.0},
                .radius = cfg.melee.radius,
            });
        registry.emplace<Attack>(m_attackEntity,
                                 Attack{.damage = cfg.melee.damage});
        HitSystem::Update(registry);
      } else if (input.rangedAttackDown) {
        m_attackClip = U"ranged_attack";
        m_attackTimer = getClipDuration(U"ranged_attack");

        const double sign = anim.facingRight ? 1.0 : -1.0;
        m_attackEntity = registry.create();
        registry.emplace<WorldPos>(m_attackEntity,
                                   registry.get<WorldPos>(m_playerRoot));
        registry.emplace<LocalOffset>(m_attackEntity, LocalOffset{});
        Hierarchy::Attach(registry, m_playerRoot, m_attackEntity);
        registry.emplace<Collider>(
            m_attackEntity,
            Collider{
                .segmentStart = Vec3{0.0, cfg.melee.capMidH, 0.0},
                .segmentEnd =
                    Vec3{sign * cfg.ranged.reach, cfg.melee.capMidH, 0.0},
                .radius = cfg.ranged.radius,
            });
        registry.emplace<Attack>(m_attackEntity,
                                 Attack{.damage = cfg.ranged.damage});
        HitSystem::Update(registry);
      }
    }
  }

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
  m_attackClip = U"";
  m_attackTimer = 0.0;
  onAfterPush(registry);
}

void PlayerTestPhase::onBeforePop(entt::registry& registry) {
  // m_attackEntity は m_playerRoot の子孫なので DestroyWithChildren
  // で連動して破棄される
  if (m_playerRoot != entt::null) {
    Hierarchy::DestroyWithChildren(registry, m_playerRoot);
    m_playerRoot = entt::null;
  }
  m_attackEntity = entt::null;
  if (m_dummyTarget != entt::null && registry.valid(m_dummyTarget)) {
    registry.destroy(m_dummyTarget);
    m_dummyTarget = entt::null;
  }
}
