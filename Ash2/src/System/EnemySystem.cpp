#include <Siv3D.hpp>

#include "System/EnemySystem.hpp"

#include "Component/EnemyMotion.hpp"
#include "Component/Hierarchy.hpp"

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
    // 敵自身に子（レティクル等）が付いていれば連動して破棄する。
    // 子を持たない敵に呼んでも registry.destroy() と同じ結果になる
    Hierarchy::DestroyWithChildren(registry, entity);
  }
}
