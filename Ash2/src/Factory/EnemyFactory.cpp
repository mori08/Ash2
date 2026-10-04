#include "Factory/EnemyFactory.hpp"

#include "Component/Boundary.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Enemy.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/Gravity.hpp"
#include "Component/Hp.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Team.hpp"
#include "Component/Velocity.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "Config/TomlFields.hpp"
#include "System/AnimationSystem.hpp"

std::expected<EnemyFactory::Param, String> EnemyFactory::Param::FromToml(
    const TOMLValue& toml
) {
  TomlFields f{toml, U"EnemyFactory::Param::FromToml"};
  return f.wrap(
      Param{
          .pos = WorldPos{.w = f.get<double>(U"w"), .d = f.get<double>(U"d")},
      }
  );
}

entt::entity EnemyFactory::Create(
    entt::registry& registry, const Param& param
) {
  const auto& enemyCfg = registry.ctx().get<EnemyConfig>();
  const auto& playerCfg = registry.ctx().get<PlayerConfig>();

  const auto enemy = registry.create();
  registry.emplace<Enemy>(enemy);
  registry.emplace<Team>(enemy, Team::Enemy);
  registry.emplace<WorldPos>(enemy, param.pos);
  registry.emplace<Velocity>(enemy);
  // Knockback の放物線は GravitySystem に任せるため、プレイヤーと同じ重力
  // 加速度を与える（EnemyConfig は専用の重力値を持たない）
  registry.emplace<Gravity>(enemy, Gravity{.accel = playerCfg.gravity});
  registry.emplace<EnemyMotion::Variant>(enemy, EnemyMotion::Idle{});
  registry.emplace<Drawable>(
      enemy, TextureDrawable{.anchor = DrawAnchor::BottomCenter}
  );
  registry.emplace<SpriteAnimation>(
      enemy, SpriteAnimation{.dataKey = U"enemy", .currentClip = U"idle"}
  );
  registry.emplace<Collider>(
      enemy,
      Collider{
          .segmentStart = Vec3{0.0, 0.0, 0.0},
          .segmentEnd = Vec3{0.0, enemyCfg.capsuleHeight, 0.0},
          .radius = enemyCfg.capsuleRadius
      }
  );
  registry.emplace<Hp>(
      enemy, Hp{.max = enemyCfg.maxHp, .current = enemyCfg.maxHp}
  );
  registry.emplace<Boundary>(enemy);
  AnimationSystem::Update(registry, 0.0);
  return enemy;
}
