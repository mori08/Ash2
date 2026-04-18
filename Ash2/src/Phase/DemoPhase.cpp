#include "Phase/DemoPhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/Player.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Input/PlayerInputAction.hpp"

void DemoPhase::onAfterPush(entt::registry& registry) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();

  auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player);
  registry.emplace<Velocity>(player);
  registry.emplace<Drawable>(
      player, RectDrawable{.size = cfg.spriteSize, .color = cfg.spriteColor});
}

IPhase::PhaseCommand DemoPhase::update(entt::registry& registry, double dt) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& actions = registry.ctx().get<PlayerInputAction>();

  const double vw = actions.moveRight.pressed()  ? cfg.speed
                    : actions.moveLeft.pressed() ? -cfg.speed
                                                 : 0.0;
  const double vd = actions.moveForward.pressed()    ? cfg.speed
                    : actions.moveBackward.pressed() ? -cfg.speed
                                                     : 0.0;

  auto view = registry.view<Player, WorldPos, Velocity>();
  for (auto [entity, pos, vel] : view.each()) {
    vel.w = vw;
    vel.d = vd;
    pos.w += vel.w * dt;
    pos.d += vel.d * dt;

    if (actions.jump.down() && pos.isOnGround()) {
      vel.h = cfg.jumpSpeed;
    }

    vel.h -= cfg.gravity * dt;
    pos.h += vel.h * dt;

    if (pos.h < 0.0) {
      pos.h = 0.0;
      vel.h = 0.0;
    }
  }

  return PhaseCommand::None();
}
