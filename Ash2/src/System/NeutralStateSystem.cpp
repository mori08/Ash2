#include <Siv3D.hpp>

#include "System/NeutralStateSystem.hpp"

#include "Component/AnimationData.hpp"
#include "Component/Attack.hpp"
#include "Component/AttackState.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/NeutralState.hpp"
#include "Component/Player.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"

namespace {

constexpr s3d::ColorF KBulletColor = {0.9, 0.9, 0.3};

/// @brief 攻撃入力の種類
enum class AttackKind : std::uint8_t { Melee, Ranged };

/// @brief Neutral → Attack 遷移対象のエンティティとその開始時のスナップショット
struct PendingAttack {
  entt::entity entity;
  AttackKind kind;
  WorldPos pos;
  bool facingRight;
};

/// @brief 指定クリップの再生時間（秒）を返す
/// @return クリップが見つからない場合は 0.0
double GetClipDuration(const AnimationData& playerData,
                       const s3d::String& clipName) {
  const auto it = playerData.clips.find(clipName);
  if (it == playerData.clips.end()) return 0.0;
  return static_cast<double>(it->second.count) / it->second.speed;
}

}  // namespace

void NeutralStateSystem::Update(entt::registry& registry,
                                const FrameData& frameData) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& input = frameData.input;
  const auto& animReg = registry.ctx().get<AnimationDataRegistry>();
  const auto& playerData = animReg.at(U"player");

  s3d::Array<PendingAttack> pending;

  auto view = registry.view<Player, WorldPos, NeutralState, SpriteAnimation>();
  for (const auto entity : view) {
    const auto& pos = view.get<WorldPos>(entity);
    const auto& anim = view.get<SpriteAnimation>(entity);

    if (!pos.isOnGround()) continue;

    if (input.attackDown) {
      pending.push_back(PendingAttack{.entity = entity,
                                      .kind = AttackKind::Melee,
                                      .pos = pos,
                                      .facingRight = anim.facingRight});
    } else if (input.rangedAttackDown) {
      pending.push_back(PendingAttack{.entity = entity,
                                      .kind = AttackKind::Ranged,
                                      .pos = pos,
                                      .facingRight = anim.facingRight});
    }
  }

  for (const auto& attack : pending) {
    const double sign = attack.facingRight ? 1.0 : -1.0;

    if (attack.kind == AttackKind::Melee) {
      const auto attackEntity = registry.create();
      registry.emplace<WorldPos>(attackEntity, attack.pos);
      registry.emplace<LocalOffset>(attackEntity, LocalOffset{});
      Hierarchy::Attach(registry, attack.entity, attackEntity);
      registry.emplace<Collider>(
          attackEntity, Collider{
                            .segmentStart = Vec3{0.0, cfg.melee.capMidH, 0.0},
                            .segmentEnd = Vec3{sign * cfg.melee.reach,
                                               cfg.melee.capMidH, 0.0},
                            .radius = cfg.melee.radius,
                        });
      registry.emplace<Attack>(attackEntity,
                               Attack{.damage = cfg.melee.damage});

      registry.erase<NeutralState>(attack.entity);
      registry.emplace<AttackState>(
          attack.entity,
          AttackState{.clip = U"attack",
                      .timer = GetClipDuration(playerData, U"attack"),
                      .entity = attackEntity});
    } else {
      const auto bullet = registry.create();
      registry.emplace<WorldPos>(
          bullet, WorldPos{.w = attack.pos.w,
                           .h = attack.pos.h + cfg.ranged.spawnHeight,
                           .d = attack.pos.d});
      registry.emplace<Velocity>(bullet,
                                 Velocity{.w = sign * cfg.ranged.bulletSpeed});
      registry.emplace<Collider>(bullet,
                                 Collider{
                                     .segmentStart = Vec3{0.0, 0.0, 0.0},
                                     .segmentEnd = Vec3{0.0, 0.0, 0.0},
                                     .radius = cfg.ranged.radius,
                                 });
      registry.emplace<Attack>(bullet, Attack{.damage = cfg.ranged.damage});
      registry.emplace<Drawable>(
          bullet,
          CircleDrawable{.radius = cfg.ranged.radius, .color = KBulletColor});
      registry.emplace<Projectile>(bullet);

      registry.erase<NeutralState>(attack.entity);
      registry.emplace<AttackState>(
          attack.entity,
          AttackState{.clip = U"ranged_attack",
                      .timer = GetClipDuration(playerData, U"ranged_attack"),
                      .entity = entt::null});
    }
  }
}
