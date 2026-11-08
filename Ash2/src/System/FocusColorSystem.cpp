#include <Siv3D.hpp>

#include "System/FocusColorSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/FocusColor.hpp"
#include "Component/Menu.hpp"

void FocusColorSystem::Update(entt::registry& registry) {
  for (auto&& [entity, focus] : registry.view<FocusColor, Focused>().each()) {
    registry.get_or_emplace<DrawColor>(entity).color = focus.focused;
  }
  for (auto&& [entity, focus] :
       registry.view<FocusColor>(entt::exclude<Focused>).each()) {
    registry.get_or_emplace<DrawColor>(entity).color = focus.normal;
  }
}
