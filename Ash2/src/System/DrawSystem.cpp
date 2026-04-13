#include "System/DrawSystem.hpp"

#include "Component/Drawable.hpp"
#include "WorldPos.hpp"

void DrawSystem::Draw(const entt::registry& registry) {
  const Vec2 cameraOffset = Scene::Center();

  struct DrawEntry {
    std::reference_wrapper<const WorldPos> pos;
    std::reference_wrapper<const Drawable> drawable;
  };

  Array<DrawEntry> entries;
  for (auto& [entity, pos, drawable] :
       registry.view<const WorldPos, const Drawable>().each()) {
    entries.push_back({std::cref(pos), std::cref(drawable)});
  }

  std::ranges::sort(entries, [](const DrawEntry& a, const DrawEntry& b) {
    return DrawOrderLess(a.pos.get(), b.pos.get());
  });

  for (const auto& entry : entries) {
    const Vec2 screenPos = cameraOffset + entry.pos.get().toScreen();
    std::visit(
        [&screenPos](const auto& shape) {
          using T = std::decay_t<decltype(shape)>;
          if constexpr (std::is_same_v<T, RectDrawable>) {
            RectF{Arg::bottomCenter(screenPos), shape.size.x, shape.size.y}
                .draw(shape.color);
          }
        },
        entry.drawable.get());
  }
}
