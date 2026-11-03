#include "System/AttachmentSystem.hpp"

#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/WorldPos.hpp"

namespace {
void Propagate(
    entt::registry& registry, const WorldPos& parentPos, entt::entity parent
) {
  for (const auto child : Hierarchy::Children(registry, parent)) {
    if (registry.all_of<WorldPos>(child)) {
      auto& pos = registry.get<WorldPos>(child);
      const auto& off = registry.get<const LocalOffset>(child);
      pos = {
          .w = parentPos.w + off.w,
          .h = parentPos.h + off.h,
          .d = parentPos.d + off.d
      };
      Propagate(registry, pos, child);
    }
  }
}
}  // namespace

void AttachmentSystem::UpdateTransform(entt::registry& registry) {
  for (const auto& [entity, node, pos] :
       registry.view<const Hierarchy, const WorldPos>().each()) {
    if (node.parent() != entt::null) {
      continue;
    }
    Propagate(registry, pos, entity);
  }
}
