#include "Phase/DemoPhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/Player.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"

void DemoPhase::onAfterPush(entt::registry& registry) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();

  auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player);
  registry.emplace<Velocity>(player);
  registry.emplace<Drawable>(
      player, RectDrawable{.size = cfg.spriteSize, .color = cfg.spriteColor});
}

IPhase::PhaseCommand DemoPhase::update(entt::registry& registry,
                                       const FrameData& frameData) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& input = frameData.input;

  const double vw = input.moveRight  ? cfg.speed
                    : input.moveLeft ? -cfg.speed
                                     : 0.0;
  const double vd = input.moveForward    ? cfg.speed
                    : input.moveBackward ? -cfg.speed
                                         : 0.0;

  auto view = registry.view<Player, WorldPos, Velocity>();
  for (auto [entity, pos, vel] : view.each()) {
    vel.w = vw;
    vel.d = vd;
    pos.w += vel.w * frameData.dt;
    pos.d += vel.d * frameData.dt;

    if (input.jumpDown && pos.isOnGround()) {
      vel.h = cfg.jumpSpeed;
    }

    vel.h -= cfg.gravity * frameData.dt;
    pos.h += vel.h * frameData.dt;

    if (pos.h < 0.0) {
      pos.h = 0.0;
      vel.h = 0.0;
    }
  }

  return PhaseCommand::None();
}
