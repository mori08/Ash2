#include <Siv3D.hpp>

#include "System/EnemySystem.hpp"

#include "Component/EnemyMotion.hpp"

void EnemySystem::Update(entt::registry& registry) {
  // view.each() の走査中に destroy() すると走査が壊れるため、破棄対象は
  // 収集してからループの外でまとめて破棄する
  Array<entt::entity> toDestroy;

  auto view = registry.view<EnemyMotion::Variant>();
  for (auto&& [entity, motion] : view.each()) {
    const auto* defeated = std::get_if<EnemyMotion::Defeated>(&motion);
    if (defeated != nullptr && defeated->remaining <= 0.0) {
      toDestroy.push_back(entity);
    }
  }

  for (const auto entity : toDestroy) {
    registry.destroy(entity);
  }
}
