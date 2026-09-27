#include "Factory/PlayerFactory.hpp"

#include "Component/Boundary.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Gravity.hpp"
#include "Component/Hp.hpp"
#include "Component/LockOn.hpp"
#include "Component/Name.hpp"
#include "Component/Player.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Stamina.hpp"
#include "Component/Team.hpp"
#include "Component/Velocity.hpp"
#include "Config/PlayerConfig.hpp"
#include "System/AnimationSystem.hpp"

namespace {
constexpr int32 kPlayerMaxHp = 100;
}

entt::entity PlayerFactory::Create(
    entt::registry& registry, const Param& param
) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();

  const auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<Team>(player, Team::Player);
  registry.emplace<WorldPos>(player, param.pos);
  registry.emplace<Velocity>(player);
  registry.emplace<Gravity>(player, Gravity{.accel = cfg.gravity});
  registry.emplace<Name>(player, Name{U"player"});
  registry.emplace<Drawable>(
      player, TextureDrawable{.anchor = DrawAnchor::BottomCenter}
  );
  registry.emplace<SpriteAnimation>(
      player, SpriteAnimation{.dataKey = U"player", .currentClip = U"idle"}
  );
  registry.emplace<Hp>(
      player, Hp{.max = kPlayerMaxHp, .current = kPlayerMaxHp}
  );
  registry.emplace<Collider>(
      player,
      Collider{
          .segmentStart = Vec3{0.0, 0.0, 0.0},
          .segmentEnd = Vec3{0.0, cfg.capsuleHeight, 0.0},
          .radius = cfg.capsuleRadius
      }
  );
  registry.emplace<Stamina>(
      player, Stamina{.max = cfg.stamina.max, .current = cfg.stamina.max}
  );
  registry.emplace<PlayerMotion::Variant>(player, PlayerMotion::Neutral{});
  registry.emplace<LockOn>(player);
  registry.emplace<Boundary>(player);
  AnimationSystem::Update(registry, 0.0);

  return player;
}
