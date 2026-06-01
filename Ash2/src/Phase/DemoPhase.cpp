#include "Phase/DemoPhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Name.hpp"
#include "Component/Player.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/AnimationSystem.hpp"
#include "System/PlayerMovementSystem.hpp"

void DemoPhase::onAfterPush(entt::registry& registry) {
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
}

IPhase::PhaseCommand DemoPhase::update(entt::registry& registry,
                                       const FrameData& frameData) {
  PlayerMovementSystem::Update(registry, frameData);

  AnimationSystem::Update(registry, frameData.dt);

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
  Hierarchy::DestroyWithChildren(registry, m_playerRoot);
  m_playerRoot = entt::null;
}
