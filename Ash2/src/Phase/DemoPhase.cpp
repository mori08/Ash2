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
  const auto& tc = cfg.texture;

  m_playerTexture = s3d::Texture{U"image/player.png"};
  const int cols = m_playerTexture.width() / tc.frameWidth;
  const int col = tc.frameIndex % cols;
  const int row = tc.frameIndex / cols;
  const s3d::TextureRegion region = m_playerTexture(
      col * tc.frameWidth, row * tc.frameHeight, tc.frameWidth, tc.frameHeight);

  m_playerRoot = registry.create();
  registry.emplace<Player>(m_playerRoot);
  registry.emplace<WorldPos>(m_playerRoot);
  registry.emplace<Velocity>(m_playerRoot);
  registry.emplace<Name>(m_playerRoot, Name{U"player"});
  registry.emplace<Drawable>(
      m_playerRoot,
      TextureDrawable{.region = region, .drawOffset = tc.drawOffset});
  registry.ctx().get<NameLookup>()[U"player"] = m_playerRoot;
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

  if (frameData.input.reloadConfig) {
    reloadPlayer(registry);
  }

  return PhaseCommand::None();
}

void DemoPhase::reloadPlayer(entt::registry& registry) {
  onBeforePop(registry);
  onAfterPush(registry);
}

void DemoPhase::onBeforePop(entt::registry& registry) {
  if (m_playerRoot == entt::null) {
    return;
  }
  registry.ctx().get<NameLookup>().erase(U"player");
  Hierarchy::DestroyWithChildren(registry, m_playerRoot);
  m_playerRoot = entt::null;
}
