#include "System/DrawSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/ScreenPos.hpp"
#include "Component/WorldPos.hpp"
#include "Screen.hpp"
#include "System/DrawShape.hpp"

void DrawSystem::Draw(const entt::registry& registry) {
  struct DrawEntry {
    DrawOrderKey order;
    std::reference_wrapper<const WorldPos> pos;
    std::reference_wrapper<const Drawable> drawable;
    ColorF color;
  };

  Array<DrawEntry> entries;
  for (const auto& [entity, pos, drawable] :
       registry.view<const WorldPos, const Drawable>(entt::exclude<ScreenPos>)
           .each()) {
    const auto* drawColor = registry.try_get<DrawColor>(entity);
    entries.push_back(
        {.order = {.d = pos.d, .entity = entity},
         .pos = std::cref(pos),
         .drawable = std::cref(drawable),
         .color = (drawColor != nullptr) ? drawColor->color : kDefaultDrawColor}
    );
  }

  std::ranges::sort(entries, DrawOrderLess, &DrawEntry::order);

  for (const auto& entry : entries) {
    DrawShape(
        entry.drawable.get(), WorldToScreen(entry.pos.get()), entry.color
    );
  }
}
