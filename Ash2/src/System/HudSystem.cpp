#include "System/HudSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/ScreenPos.hpp"
#include "System/DrawOne.hpp"

namespace {

/// @brief ScreenPos + Drawable を (layer, entity) の昇順で描画する
///
/// storage の走査順は削除で入れ替わるため、ソートして毎フレーム同じ順にする。
/// layer が同値のものは entity 昇順にする。
void DrawScreenEntities(const entt::registry& registry) {
  struct Entry {
    entt::entity entity;
    std::reference_wrapper<const ScreenPos> pos;
    std::reference_wrapper<const Drawable> drawable;
    ColorF color;
  };

  Array<Entry> entries;
  for (const auto& [entity, pos, drawable] :
       registry.view<const ScreenPos, const Drawable>().each()) {
    const auto* drawColor = registry.try_get<DrawColor>(entity);
    entries.push_back(
        {.entity = entity,
         .pos = std::cref(pos),
         .drawable = std::cref(drawable),
         .color = (drawColor != nullptr) ? drawColor->color : kDefaultDrawColor}
    );
  }

  std::ranges::sort(entries, {}, [](const Entry& entry) {
    return std::pair{entry.pos.get().layer, entry.entity};
  });

  for (const auto& entry : entries) {
    DrawOne(entry.drawable.get(), entry.pos.get().pos, entry.color);
  }
}

}  // namespace

void HudSystem::Draw(const entt::registry& registry) {
  DrawScreenEntities(registry);
}
