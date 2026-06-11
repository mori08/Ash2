#include <Siv3D.hpp>

#include "System/AttackStateSystem.hpp"

#include "Component/AttackState.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/NeutralState.hpp"
#include "Component/Player.hpp"
#include "Component/WorldPos.hpp"
#include "Phase/FrameData.hpp"

void AttackStateSystem::Update(entt::registry& registry,
                               const FrameData& frameData) {
  const double dt = frameData.dt;

  s3d::Array<entt::entity> finished;

  auto view = registry.view<Player, WorldPos, AttackState>();
  for (const auto entity : view) {
    auto& attackState = view.get<AttackState>(entity);
    attackState.timer -= dt;
    if (attackState.timer <= 0.0) {
      finished.push_back(entity);
    }
  }

  for (const auto entity : finished) {
    const auto attackEntity = registry.get<AttackState>(entity).entity;
    registry.erase<AttackState>(entity);
    registry.emplace<NeutralState>(entity);

    if (attackEntity != entt::null) {
      Hierarchy::DestroyWithChildren(registry, attackEntity);
    }
  }
}
