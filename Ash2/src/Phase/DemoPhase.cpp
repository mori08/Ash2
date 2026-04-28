#include "Phase/DemoPhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Name.hpp"
#include "Component/Player.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/NameLookup.hpp"

void DemoPhase::onAfterPush(entt::registry& registry) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();

  m_playerRoot = registry.create();
  registry.emplace<Player>(m_playerRoot);
  registry.emplace<WorldPos>(m_playerRoot);
  registry.emplace<Velocity>(m_playerRoot);
  registry.emplace<Name>(m_playerRoot, Name{U"player"});
  registry.ctx().get<NameLookup>()[U"player"] = m_playerRoot;

  const auto makeCircle = [&](const PlayerCirclePartConfig& part) {
    auto e = registry.create();
    registry.emplace<WorldPos>(e);
    registry.emplace<Drawable>(
        e, CircleDrawable{.radius = part.radius, .color = part.color});
    Hierarchy::Attach(registry, m_playerRoot, e, part.offset);
  };

  auto body = registry.create();
  registry.emplace<WorldPos>(body);
  registry.emplace<Drawable>(body,
                             PieDrawable{.radius = cfg.body.radius,
                                         .startAngle = cfg.body.startAngle,
                                         .angle = cfg.body.angle,
                                         .color = cfg.body.color});
  Hierarchy::Attach(registry, m_playerRoot, body, cfg.body.offset);

  makeCircle(cfg.head);
  makeCircle(cfg.handFront);
  makeCircle(cfg.handBack);
  makeCircle(cfg.footLeft);
  makeCircle(cfg.footRight);
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

void DemoPhase::onBeforePop(entt::registry& registry) {
  if (m_playerRoot == entt::null) {
    return;
  }
  registry.ctx().get<NameLookup>().erase(U"player");
  Hierarchy::DestroyWithChildren(registry, m_playerRoot);
  m_playerRoot = entt::null;
}
