#include "Component/Hierarchy.hpp"

#include "Component/LocalOffset.hpp"

void Hierarchy::Attach(entt::registry& registry, entt::entity parent,
                       entt::entity child, WorldPos offset) {
  if (!registry.all_of<Hierarchy>(parent)) {
    registry.emplace<Hierarchy>(parent);
  }
  if (!registry.all_of<Hierarchy>(child)) {
    registry.emplace<Hierarchy>(child);
  }

  auto& parentNode = registry.get<Hierarchy>(parent);
  auto& childNode = registry.get<Hierarchy>(child);

  childNode.m_parent = parent;
  childNode.m_nextSibling = parentNode.m_firstChild;
  if (parentNode.m_firstChild != entt::null) {
    registry.get<Hierarchy>(parentNode.m_firstChild).m_prevSibling = child;
  }
  parentNode.m_firstChild = child;

  registry.emplace_or_replace<LocalOffset>(child, offset);
}

void Hierarchy::Detach(entt::registry& registry, entt::entity child) {
  if (!registry.all_of<Hierarchy>(child)) {
    return;
  }

  auto& childNode = registry.get<Hierarchy>(child);

  if (childNode.m_parent != entt::null && registry.valid(childNode.m_parent)) {
    if (childNode.m_prevSibling != entt::null) {
      registry.get<Hierarchy>(childNode.m_prevSibling).m_nextSibling =
          childNode.m_nextSibling;
    } else {
      registry.get<Hierarchy>(childNode.m_parent).m_firstChild =
          childNode.m_nextSibling;
    }
  }

  if (childNode.m_nextSibling != entt::null) {
    registry.get<Hierarchy>(childNode.m_nextSibling).m_prevSibling =
        childNode.m_prevSibling;
  }

  childNode.m_parent = entt::null;
  childNode.m_prevSibling = entt::null;
  childNode.m_nextSibling = entt::null;

  registry.remove<LocalOffset>(child);
}

void Hierarchy::DestroyWithChildren(entt::registry& registry,
                                    entt::entity entity) {
  if (registry.all_of<Hierarchy>(entity)) {
    auto child = registry.get<const Hierarchy>(entity).m_firstChild;
    while (child != entt::null) {
      const auto next = registry.get<const Hierarchy>(child).m_nextSibling;
      DestroyWithChildren(registry, child);
      child = next;
    }
  }
  registry.destroy(entity);
}
