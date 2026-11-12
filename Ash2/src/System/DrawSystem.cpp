#include "System/DrawSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"
#include "System/DrawOne.hpp"

void DrawSystem::Draw(const entt::registry& registry) {
  struct DrawEntry {
    DrawOrderKey order;
    std::reference_wrapper<const DrawPos> pos;
    std::reference_wrapper<const Drawable> drawable;
    ColorF color;
  };

  Array<DrawEntry> entries;
  for (const auto& [entity, pos, drawable] :
       registry.view<const DrawPos, const Drawable>().each()) {
    const auto* drawColor = registry.try_get<DrawColor>(entity);
    entries.push_back(
        {.order = {.layer = pos.layer, .depth = pos.depth, .entity = entity},
         .pos = std::cref(pos),
         .drawable = std::cref(drawable),
         .color = (drawColor != nullptr) ? drawColor->color : kDefaultDrawColor}
    );
  }

  // storage の走査順は削除で入れ替わるため、ソートして毎フレーム同じ順にする
  std::ranges::sort(entries, DrawOrderLess, &DrawEntry::order);

  for (const auto& entry : entries) {
    DrawOne(entry.drawable.get(), entry.pos.get().pos, entry.color);
  }
}
