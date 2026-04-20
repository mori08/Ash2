#include "System/AttachmentSystem.hpp"

#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/WorldPos.hpp"

namespace {
void Propagate(entt::registry& registry, const WorldPos& parentPos,
               entt::entity child) {
  while (child != entt::null) {
    const auto& node = registry.get<const Hierarchy>(child);
    if (registry.all_of<WorldPos>(child)) {
      auto& pos = registry.get<WorldPos>(child);
      if (registry.all_of<LocalOffset>(child)) {
        const auto& off = registry.get<const LocalOffset>(child).value;
        pos = {.w = parentPos.w + off.w,
               .h = parentPos.h + off.h,
               .d = parentPos.d + off.d};
      } else {
        pos = parentPos;
      }
      Propagate(registry, pos, node.firstChild());
    }
    child = node.nextSibling();
  }
}
}  // namespace

void AttachmentSystem::UpdateTransform(entt::registry& registry) {
  for (const auto& [entity, node, pos] :
       registry.view<const Hierarchy, const WorldPos>().each()) {
    if (node.parent() != entt::null) {
      continue;
    }
    Propagate(registry, pos, node.firstChild());
  }
}
